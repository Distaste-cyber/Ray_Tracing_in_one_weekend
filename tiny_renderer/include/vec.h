#ifndef VEC_H
#define VEC_H

#include "random.h"
#include <cassert>
#include <cmath>
#include <initializer_list>
#include <iostream>
template <size_t N> class vec {
public:
  double e[N]{};

  vec() = default;

  vec(std::initializer_list<double> list) {
    assert(list.size() == N);
    size_t i = 0;
    for (double item : list)
      e[i++] = item;
  }

  double &operator[](size_t i) { return e[i]; }
  const double &operator[](size_t i) const { return e[i]; }

  vec operator-() const {
    vec r;
    for (size_t i = 0; i < N; i++)
      r.e[i] = -e[i];
    return r;
  }

  vec &operator+=(const vec &v) {
    for (size_t i = 0; i < N; i++)
      e[i] += v.e[i];
    return *this;
  }

  vec &operator*=(double t) {
    for (size_t i = 0; i < N; i++)
      e[i] *= t;
    return *this;
  }

  vec &operator/=(double t) { return *this *= (1.0 / t); }

  double length_squared() const {
    double sum = 0;
    for (size_t i = 0; i < N; i++)
      sum += e[i] * e[i];
    return sum;
  }

  double length() const { return std::sqrt(length_squared()); }

  static vec random() {
    vec r;
    for (size_t i = 0; i < N; i++)
      r.e[i] = random_double();
    return r;
  }

  static vec random(double min, double max) {
    vec r;
    for (size_t i = 0; i < N; i++)
      r.e[i] = random_double(min, max);
    return r;
  }

  static vec random_unit_vector() {
    while (true) {
      auto p = vec<N>::random(-1, 1);
      auto lensq = p.length_squared();

      if (1e-12 <= lensq && lensq <= 1) {
        return p / sqrt(lensq);
      }
    }
  }
};

template <size_t N> inline vec<N> operator+(const vec<N> &a, const vec<N> &b) {
  vec<N> r;
  for (size_t i = 0; i < N; ++i)
    r[i] = a[i] + b[i];
  return r;
}

template <size_t N> inline vec<N> operator-(const vec<N> &a, const vec<N> &b) {
  vec<N> r;
  for (size_t i = 0; i < N; ++i)
    r[i] = a[i] - b[i];
  return r;
}

template <size_t N> inline vec<N> operator*(const vec<N> &v, double t) {
  vec<N> r;
  for (size_t i = 0; i < N; ++i)
    r[i] = v[i] * t;
  return r;
}

template <size_t N> inline vec<N> operator*(double t, const vec<N> &v) {
  return v * t;
}

template <size_t N> inline vec<N> operator/(const vec<N> &v, double t) {
  return v * (1.0 / t);
}

template <size_t N> inline double dot(const vec<N> &a, const vec<N> &b) {
  double sum = 0;
  for (size_t i = 0; i < N; ++i)
    sum += a[i] * b[i];
  return sum;
}

template <size_t N>
inline std::ostream &operator<<(std::ostream &out, const vec<N> &v) {
  for (size_t i = 0; i < N; ++i) {
    out << v[i];
    if (i + 1 < N)
      out << " ";
  }
  return out;
}

inline vec<3> cross(const vec<3> &a, const vec<3> &b) {
  return vec<3>{a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2],
                a[0] * b[1] - a[1] * b[0]};
}
template <size_t N> inline vec<N> unit_vector(const vec<N> &v) {
  auto len = v.length();
  if (len < 1e-16)
    return vec<N>(); // avoid NaNs
  return v / len;
}

template <size_t N> inline vec<N> random_on_hemisphere(const vec<N> &normal) {
  vec<N> on_unit_sphere = vec<N>::random_unit_vector();
  if (dot(on_unit_sphere, normal) > 0.0) {
    return on_unit_sphere;
  } else {
    return -on_unit_sphere;
  }
}
template <size_t N> inline vec<N> reflect(const vec<N> &v, const vec<N> &n) {
  return v - 2 * dot(v, n) * n;
}
template <size_t N>
inline vec<N> refract(const vec<N> &uv, const vec<N> &n,
                      double etai_over_etat) {
  auto cos_theta = std::fmin(dot(-uv, n), 1.0);
  vec<N> r_out_perp = etai_over_etat * (uv + cos_theta * n);
  vec<N> r_out_parallel =
      -std::sqrt(std::fabs(1.0 - r_out_perp.length_squared())) * n;
  return r_out_perp + r_out_parallel;
}

template <size_t N> double norm(vec<N> v) {
  double norm = 0;
  for (size_t i = 0; i < N; i++) {
    norm += v[i] * v[i];
  }

  norm = std::sqrt(norm);
  return norm;
};

#endif
