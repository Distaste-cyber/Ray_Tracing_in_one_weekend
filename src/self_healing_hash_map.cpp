// error_healing_hashmap.h
#pragma once
#include <array>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <functional>
#include <optional>
#include <vector>

// Small CRC16 (CCITT) implementation
static uint16_t crc16_ccitt(const void *data, size_t len,
                            uint16_t seed = 0xFFFF) {
  const uint8_t *p = (const uint8_t *)data;
  uint16_t crc = seed;
  while (len--) {
    crc ^= (uint16_t)(*p++) << 8;
    for (int k = 0; k < 8; ++k) {
      crc = (crc & 0x8000) ? (crc << 1) ^ 0x1021 : (crc << 1);
    }
  }
  return crc;
}

// Simple hash to 32-bit (user may supply better)
static uint32_t simple_hash_u32(uint32_t x) {
  x ^= x >> 16;
  x *= 0x7feb352d;
  x ^= x >> 15;
  x *= 0x846ca68b;
  x ^= x >> 16;
  return x;
}

// Template: Key is uint32_t ID; ValueBytes is fixed bytes for value
template <size_t Capacity, size_t ValueBytes, size_t GroupSize = 8>
class ErrorHealingHashMap {
  static_assert(Capacity % GroupSize == 0,
                "Capacity must be divisible by GroupSize");

public:
  // POD value storage type
  struct Value {
    std::array<uint8_t, ValueBytes> data;
  };

private:
  enum SlotState : uint8_t { FREE = 0, USED = 1, TOMBSTONE = 2 };

  struct Slot {
    uint32_t key;  // 4 bytes (could be 2 if you need)
    uint8_t fp;    // 1 byte fingerprint
    uint16_t crc;  // 2 bytes checksum
    uint8_t state; // 1 byte
    Value value;   // ValueBytes bytes
    // padding depends on alignment; pack with pragma if needed for exact size
  };

  // Group parity metadata
  struct GroupMeta {
    Value xor_values;  // XOR of all Value.data in group
    uint32_t xor_keys; // XOR of all keys in group
    uint16_t xor_crcs; // XOR of all crcs (optional)
    bool initialized;
    GroupMeta() : xor_keys(0), xor_crcs(0), initialized(false) {
      xor_values.data.fill(0);
    }
  };

  std::array<Slot, Capacity> slots;
  std::array<GroupMeta, Capacity / GroupSize> groups;

  // hash function -> index
  inline uint32_t index_for_key(uint32_t key) const {
    uint32_t h = simple_hash_u32(key);
    return h % Capacity;
  }

  inline uint8_t fingerprint(uint32_t key) const {
    uint32_t h = simple_hash_u32(key);
    return uint8_t((h >> 8) & 0xFF);
  }

  // compute CRC over key + value
  inline uint16_t compute_crc(const Slot &s) const {
    // combine key and value bytes into CRC
    uint8_t buffer[sizeof(uint32_t) + ValueBytes];
    std::memcpy(buffer, &s.key, sizeof(uint32_t));
    std::memcpy(buffer + sizeof(uint32_t), s.value.data.data(), ValueBytes);
    return crc16_ccitt(buffer, sizeof(buffer));
  }

  // group index
  inline size_t group_of_index(size_t idx) const { return idx / GroupSize; }

  // XOR helpers
  static void xor_values(Value &dest, const Value &a, const Value &b) {
    for (size_t i = 0; i < ValueBytes; ++i)
      dest.data[i] = a.data[i] ^ b.data[i];
  }
  static void xor_into(Value &dest, const Value &src) {
    for (size_t i = 0; i < ValueBytes; ++i)
      dest.data[i] ^= src.data[i];
  }

public:
  ErrorHealingHashMap() {
    // mark free
    for (size_t i = 0; i < Capacity; ++i) {
      slots[i].state = FREE;
      // optional: slots[i].key = 0;
    }
    for (auto &g : groups) {
      g = GroupMeta();
      g.initialized = true;
    }
  }

  // Insert or update. Returns true if inserted or updated.
  bool put(uint32_t key, const uint8_t value_bytes[ValueBytes]) {
    uint32_t idx = index_for_key(key);
    uint8_t fp = fingerprint(key);

    // Linear probing
    size_t first_tombstone = SIZE_MAX;
    for (size_t i = 0; i < Capacity; ++i) {
      size_t pos = (idx + i) % Capacity;
      Slot &s = slots[pos];

      if (s.state == FREE) {
        // use tombstone if found earlier
        size_t usepos = (first_tombstone != SIZE_MAX) ? first_tombstone : pos;
        return place_into_slot(usepos, key, fp, value_bytes);
      }

      if (s.state == TOMBSTONE && first_tombstone == SIZE_MAX) {
        first_tombstone = pos;
      }

      if (s.state == USED && s.fp == fp && s.key == key) {
        // update existing
        return place_into_slot(pos, key, fp, value_bytes);
      }
    }
    // table full (no free)
    return false;
  }

  std::optional<std::array<uint8_t, ValueBytes>> get(uint32_t key) {
    uint32_t idx = index_for_key(key);
    uint8_t fp = fingerprint(key);

    for (size_t i = 0; i < Capacity; ++i) {
      size_t pos = (idx + i) % Capacity;
      Slot &s = slots[pos];
      if (s.state == FREE)
        return std::nullopt; // not found
      if (s.state == USED && s.fp == fp && s.key == key) {
        // verify CRC
        uint16_t crc = compute_crc(s);
        if (crc == s.crc) {
          std::array<uint8_t, ValueBytes> out;
          std::memcpy(out.data(), s.value.data.data(), ValueBytes);
          return out;
        } else {
          // attempt repair
          if (attempt_repair_group(pos)) {
            // after repair, re-validate
            if (slots[pos].state == USED &&
                compute_crc(slots[pos]) == slots[pos].crc) {
              std::array<uint8_t, ValueBytes> out;
              std::memcpy(out.data(), slots[pos].value.data.data(), ValueBytes);
              return out;
            }
          }
          return std::nullopt; // corrupted
        }
      }
    }
    return std::nullopt;
  }

  bool erase(uint32_t key) {
    uint32_t idx = index_for_key(key);
    uint8_t fp = fingerprint(key);
    for (size_t i = 0; i < Capacity; ++i) {
      size_t pos = (idx + i) % Capacity;
      Slot &s = slots[pos];
      if (s.state == FREE)
        return false;
      if (s.state == USED && s.fp == fp && s.key == key) {
        // remove and update group parity
        size_t gidx = group_of_index(pos);
        xor_into(groups[gidx].xor_values, s.value);
        groups[gidx].xor_keys ^= s.key;
        groups[gidx].xor_crcs ^= s.crc;
        s.state = TOMBSTONE;
        return true;
      }
    }
    return false;
  }

  // scan whole table and try to repair groups
  void scrub_and_repair_all() {
    size_t num_groups = Capacity / GroupSize;
    for (size_t gi = 0; gi < num_groups; ++gi) {
      attempt_repair_group_by_index(gi);
    }
  }

private:
  // Place key/value into a slot (used for insert or update).
  bool place_into_slot(size_t pos, uint32_t key, uint8_t fp,
                       const uint8_t value_bytes[ValueBytes]) {
    Slot &s = slots[pos];

    // if occupied, we will XOR out old and XOR in new; else XOR in new
    Value oldVal;
    uint32_t oldKey = 0;
    uint16_t oldCrc = 0;
    bool wasUsed = (s.state == USED);
    if (wasUsed) {
      oldVal = s.value;
      oldKey = s.key;
      oldCrc = s.crc;
    }

    // set fields (write value then crc to reduce chance of partial writes
    // effect)
    std::memcpy(s.value.data.data(), value_bytes, ValueBytes);
    s.key = key;
    s.fp = fp;
    s.crc = compute_crc(s);
    s.state = USED;

    // update group parity
    size_t gidx = group_of_index(pos);
    if (!groups[gidx].initialized) {
      groups[gidx].xor_values.data.fill(0);
      groups[gidx].xor_keys = 0;
      groups[gidx].xor_crcs = 0;
      groups[gidx].initialized = true;
    }
    if (wasUsed) {
      xor_into(groups[gidx].xor_values, oldVal); // remove old
      groups[gidx].xor_keys ^= oldKey;
      groups[gidx].xor_crcs ^= oldCrc;
    }
    xor_into(groups[gidx].xor_values, s.value); // add new
    groups[gidx].xor_keys ^= s.key;
    groups[gidx].xor_crcs ^= s.crc;
    return true;
  }

  // Attempt to repair the group containing slot pos (returns true if repaired)
  bool attempt_repair_group(size_t pos) {
    const size_t gi = group_of_index(pos);
    return attempt_repair_group_by_index(gi);
  }

  // Attempt to repair group gi. Strategy:
  // - count slots with CRC mismatch
  // - if exactly one mismatch, reconstruct that slot using group XOR parity:
  //     reconstructed_value = group_xor_values XOR XOR(all other slot values)
  //   also reconstruct key and crc via xor_keys/xor_crcs if needed.
  bool attempt_repair_group_by_index(size_t gi) {
    size_t base = gi * GroupSize;
    std::vector<size_t> bad_slots;
    // compute current XOR of present slots
    Value currXor;
    currXor.data.fill(0);
    uint32_t currXorKeys = 0;
    uint16_t currXorCrcs = 0;
    size_t usedCount = 0;

    for (size_t i = 0; i < GroupSize; ++i) {
      Slot &s = slots[base + i];
      if (s.state == USED) {
        ++usedCount;
        xor_into(currXor, s.value);
        currXorKeys ^= s.key;
        currXorCrcs ^= s.crc;
        uint16_t crcNow = compute_crc(s);
        if (crcNow != s.crc) {
          bad_slots.push_back(base + i);
        }
      }
    }

    if (bad_slots.empty())
      return false; // nothing to repair
    if (bad_slots.size() > 1)
      return false; // parity can fix only single-slot faults

    size_t bad = bad_slots[0];
    // parity-based reconstruction:
    Value reconstructed = groups[gi].xor_values;
    // XOR out all other slot values:
    for (size_t i = 0; i < GroupSize; ++i) {
      size_t idx = base + i;
      if (idx == bad)
        continue;
      Slot &s = slots[idx];
      if (s.state == USED)
        xor_into(reconstructed, s.value);
    }
    // reconstruct key and crc too (only valid if entire key bytes XOR parity
    // used)
    uint32_t reconstructed_key = groups[gi].xor_keys;
    for (size_t i = 0; i < GroupSize; ++i) {
      size_t idx = base + i;
      if (idx == bad)
        continue;
      Slot &s = slots[idx];
      if (s.state == USED)
        reconstructed_key ^= s.key;
    }
    uint16_t reconstructed_crc = groups[gi].xor_crcs;
    for (size_t i = 0; i < GroupSize; ++i) {
      size_t idx = base + i;
      if (idx == bad)
        continue;
      Slot &s = slots[idx];
      if (s.state == USED)
        reconstructed_crc ^= s.crc;
    }

    // write back to slot
    Slot &target = slots[bad];
    std::memcpy(target.value.data.data(), reconstructed.data.data(),
                ValueBytes);
    target.key = reconstructed_key;
    target.fp = fingerprint(reconstructed_key);
    target.crc =
        compute_crc(target); // compute actual CRC over reconstructed key+value
    target.state = USED;

    // update group meta to be consistent
    groups[gi].xor_values = groups[gi].xor_values; // unchanged ideally
    // ensure group's crcs and keys consistent (recompute group parity to be
    // safe) recompute from scratch for robustness
    Value recomputedX;
    recomputedX.data.fill(0);
    uint32_t recomputedKeys = 0;
    uint16_t recomputedCrcs = 0;
    for (size_t i = 0; i < GroupSize; ++i) {
      Slot &s = slots[base + i];
      if (s.state == USED) {
        xor_into(recomputedX, s.value);
        recomputedKeys ^= s.key;
        recomputedCrcs ^= s.crc;
      }
    }
    groups[gi].xor_values = recomputedX;
    groups[gi].xor_keys = recomputedKeys;
    groups[gi].xor_crcs = recomputedCrcs;
    return true;
  }
};
