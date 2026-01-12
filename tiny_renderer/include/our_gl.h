#ifndef OUR_GL_H
#define OUR_GL_H

#include "matrix.h"
#include "tgaimage.h"
#include "vec.h"
#include <vector>

void lookat(const vec<3> eye, const vec<3> center, const vec<3> up);
void init_perspective(const double f);
void init_viewport(const int x, const int y, const int w, const int h);
void init_zbuffer(const int width, const int height);

struct IShader {
  virtual std::pair<bool, TGAColor> fragment(const vec<3> bar) const = 0;
};

typedef vec<4>
    Triangle[3]; // a triangle primitive is made of three ordered points
void rasterize(const Triangle &clip, const IShader &shader,
               TGAImage &framebuffer);

#endif
