#ifndef SPHERE_H
#define SPHERE_H

#include "hittable.h"

class sphere : public hittable {
public:
  sphere(const point3 &center, double radius, std::shared_ptr<material> mat)
      : center(center), radius(radius), mat(mat) {}

  bool hit(const ray &r, interval ray_t, hit_record &rec) const override {
    vec3 oc = r.origin() - center;
    auto a = r.direction().length_squared();
    auto half_b = dot(r.direction(), oc); // half-b form
    auto c = oc.length_squared() - radius * radius;
    auto discriminant = half_b * half_b - a * c;

    if (discriminant < 0)
      return false;
    auto sqrtd = std::sqrt(discriminant);

    // Find the nearest root within the valid range
    auto root = (-half_b - sqrtd) / a;
    if (!ray_t.surrounds(root)) { // <-- correct spelling: surrounds
      root = (-half_b + sqrtd) / a;
      if (!ray_t.surrounds(root))
        return false;
    }

    rec.t = root;
    rec.p = r.at(rec.t);
    vec3 outward_normal = (rec.p - center) / radius;
    rec.set_front_face(r, outward_normal);

    // DEBUG: warn if the sphere was created without a material
    if (!mat) {
      std::clog << "Warning: sphere at " << center << " has null material.\n";
    }
    rec.mat = mat;

    return true;
  }

private:
  point3 center;
  double radius;
  std::shared_ptr<material> mat;
};

#endif
