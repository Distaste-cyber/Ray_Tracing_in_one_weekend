#include "our_gl.h"
#include <algorithm>

matrix<4> ModelView, Viewport, Perspective;
std::vector<double> zbuffer;

vec<3> normalized(const vec<3> &v) {
  double len2 = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
  if (len2 < 1e-16)
    return vec<3>{0, 0, 0};
  vec<3> r = v / std::sqrt(len2);
  return r;
}

void lookat(const vec<3> eye, const vec<3> center, const vec<3> up) {
  vec<3> n = normalized(eye - center);
  vec<3> l = normalized(cross(up, n));
  vec<3> m = normalized(cross(n, l));
  ModelView = matrix<4>{{{l[0], l[1], l[2], 0},
                         {m[0], m[1], m[2], 0},
                         {n[0], n[1], n[2], 0},
                         {0, 0, 0, 1}}} *
              matrix<4>{{1, 0, 0, -eye[0]},
                        {0, 1, 0, -eye[1]},
                        {0, 0, 1, -eye[2]},
                        {0, 0, 0, 1}};
}

void init_perspective(const double f) {
  Perspective = {{{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, -1 / f, 1}}};
}

void init_viewport(const int x, const int y, const int w, const int h) {
  Viewport = {{{w / 2., 0, 0, x + w / 2.},
               {0, h / 2., 0, y + h / 2.},
               {0, 0, 1, 0},
               {0, 0, 0, 1}}};
};

void init_zbuffer(const int width, const int height) {
  zbuffer = std::vector(width * height, -1e9);
}

inline double edge_function(const vec<2> &a, const vec<2> &b, const vec<2> &c) {
  return (c[0] - a[0]) * (b[1] - a[1]) - (c[1] - a[1]) * (b[0] - a[0]);
}

double signed_triangle_area(int ax, int ay, int bx, int by, int cx, int cy) {
  return .5 * ((by - ay) * (bx + ax) + (cy - by) * (cx + bx) +
               (ay - cy) * (ax + cx));
}

void rasterize(const Triangle &clip, const IShader &shader,
               TGAImage &framebuffer) {

  vec<3> ndc[3];
  for (int i = 0; i < 3; i++) {
    ndc[i] = vec<3>{clip[i][0] / clip[i][3], clip[i][1] / clip[i][3],
                    clip[i][2] / clip[i][3]};
  }

  vec<2> screen[3];
  for (int i = 0; i < 3; i++) {
    screen[i] = vec<2>{(ndc[i][0] + 1.0) * framebuffer.width() * 0.5,
                       (ndc[i][1] + 1.0) * framebuffer.height() * 0.5};
  }

  matrix<3> ABC = {{{screen[0][0], screen[0][1], 1.},
                    {screen[1][0], screen[1][1], 1.},
                    {screen[2][0], screen[2][1], 1.}}};
  if (ABC.determinant() <= 1)
    return;

  int minx = std::max(
      0, (int)std::floor(std::min({screen[0][0], screen[1][0], screen[2][0]})));
  int maxx = std::min(
      framebuffer.width() - 1,
      (int)std::ceil(std::max({screen[0][0], screen[1][0], screen[2][0]})));

  int miny = std::max(
      0, (int)std::floor(std::min({screen[0][1], screen[1][1], screen[2][1]})));
  int maxy = std::min(
      framebuffer.height() - 1,
      (int)std::ceil(std::max({screen[0][1], screen[1][1], screen[2][1]})));

  double area = edge_function(screen[0], screen[1], screen[2]);
  if (std::abs(area) < 1e-6)
    return; // degenerate triangle

  // ---- Raster loop
  for (int y = miny; y <= maxy; y++) {
    for (int x = minx; x <= maxx; x++) {

      vec<2> p{(double)x + 0.5, (double)y + 0.5};

      double w0 = edge_function(screen[1], screen[2], p);
      double w1 = edge_function(screen[2], screen[0], p);
      double w2 = edge_function(screen[0], screen[1], p);

      if ((w0 < 0 || w1 < 0 || w2 < 0) && (w0 > 0 || w1 > 0 || w2 > 0))
        continue;

      w0 /= area;
      w1 /= area;
      w2 /= area;

      double z = w0 * ndc[0][2] + w1 * ndc[1][2] + w2 * ndc[2][2];

      int idx = x + y * framebuffer.width();
      if (z < zbuffer[idx])
        continue;
      vec<3> bc = ABC.transpose().inverse() *
                  vec<3>{static_cast<double>(x), static_cast<double>(y), 1.};
      auto [discard, color] = shader.fragment(bc);
      if (discard)
        continue;
      zbuffer[idx] = z;
      framebuffer.set(x, y, color);
    }
  }
}
