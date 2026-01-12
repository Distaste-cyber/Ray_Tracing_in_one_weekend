#include "matrix.h"
#include "our_gl.h"
#include "tgaimage.h"
#include "vec.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <tuple>
#include <vector>

constexpr int WIDTH = 800;
constexpr int HEIGHT = 800;

constexpr TGAColor white = {255, 255, 255, 255};
constexpr TGAColor green = {0, 255, 0, 255};
constexpr TGAColor red = {0, 0, 255, 255};
constexpr TGAColor blue = {255, 128, 64, 255};
constexpr TGAColor yellow = {0, 200, 255, 255};

extern matrix<4> ModelView, Perspective; // "OpenGL" state matrices and
extern std::vector<double> zbuffer;

struct face {
  int v1, v2, v3;
};

struct RandomShader : IShader {
  const std::vector<vec<3>> &va;
  const std::vector<face> &fa;
  TGAColor color = {};
  vec<3> tri[3]; // triangle in eye coordinates

  RandomShader(const std::vector<vec<3>> &v, const std::vector<face> &f)
      : va(v), fa(f) {}

  virtual vec<4> vertex(const int iface, const int nthvert) {
    int vid;
    if (nthvert == 0)
      vid = fa[iface].v1;

    if (nthvert == 1)
      vid = fa[iface].v2;

    if (nthvert == 2)
      vid = fa[iface].v3;
    vec<3> v = va[vid];
    // current vertex in object coordinates
    vec<4> gl_Position = ModelView * vec<4>{{v[0], v[1], v[2], 1.}};
    tri[nthvert] = vec<3>{gl_Position[0], gl_Position[1],
                          gl_Position[2]}; // in eye coordinates
    return Perspective * gl_Position;      // in clip coordinates
  }

  virtual std::pair<bool, TGAColor> fragment(const vec<3> bar) const {
    return {false, color}; // do not discard the pixel
  }
};

int main() {

  std::vector<vec<3>> vertices;
  std::vector<face> faces;

  std::ifstream file("../obj/diablo_poses/diablo_pose.obj");
  std::string line;

  if (!file) {
    std::cerr << "Failed to open OBJ file\n";
    return 1;
  }

  while (std::getline(file, line)) {
    if (line.empty())
      continue;

    std::stringstream ss(line);
    std::string prefix;
    ss >> prefix;

    if (prefix == "v") {
      double x, y, z;
      ss >> x >> y >> z;
      if (!ss.fail()) {
        vec<3> v = vec<3>({x, y, z});
        vertices.push_back(v);
      }
    } else if (prefix == "f") {
      std::string token;
      std::vector<int> fv;

      while (ss >> token) {
        size_t pos = token.find('/');
        int idx =
            std::stoi(token.substr(0, pos)) - 1; // FIXED: convert to 0-based
        fv.push_back(idx);
      }

      for (size_t i = 1; i + 1 < fv.size(); i++)
        faces.push_back({fv[0], fv[i], fv[i + 1]});
    }
  }

  constexpr int width = 800; // output image size
  constexpr int height = 800;
  vec<3> eye{-1, 0, 2};   // camera position
  vec<3> center{0, 0, 0}; // camera direction
  vec<3> up{0, 1, 0};     // camera up vector

  lookat(eye, center, up);              // build the ModelView   matrix
  init_perspective(norm(eye - center)); // build the Perspective matrix
  init_viewport(width / 16, height / 16, width * 7 / 8,
                height * 7 / 8); // build the Viewport    matrix
  init_zbuffer(width, height);
  TGAImage framebuffer(width, height, TGAImage::RGB, {177, 195, 209, 255});
  RandomShader shader(vertices, faces);
  // Render all triangles
  for (int i = 0; i < faces.size(); i++) {
    const face &f = faces[i];

    shader.color = {static_cast<uint8_t>(std::rand() % 255),
                    static_cast<uint8_t>(std::rand() % 255),
                    static_cast<uint8_t>(std::rand() % 255), 255};

    Triangle clip = {shader.vertex(i, 0), shader.vertex(i, 1),
                     shader.vertex(i, 2)};

    rasterize(clip, shader, framebuffer);
  }

  framebuffer.write_tga_file("framebuffer.tga");
  return 0;
};
