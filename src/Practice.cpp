#include <iostream>
#include <vector>
#define LOG(x) std::cout << x << std::endl
struct Vertex {
  int x, y, z;
};

std::ostream &operator<<(std::ostream &stream, const Vertex &v) {
  stream << v.x << "," << v.y << "," << v.z;
  return stream;
}

int main() {
  std::vector<Vertex> vertices;

  vertices.push_back({1, 2, 3});
  vertices.push_back({6, 1, 5});

  for (Vertex &v : vertices) {
    LOG(v);
  }

  return 0;
}