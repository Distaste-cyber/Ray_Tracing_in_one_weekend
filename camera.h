#ifndef CAMERA_H
#define CAMERA_H

#include "hittable.h"
#include "material.h"
#include "rtweekend.h"
#include "vec3.h"

class camera {
public:
  double aspect_ratio = 1.0;
  int width = 100;
  int samples_per_pixel = 10;
  int max_depth = 10;

  double vfov = 90;
  point3 lookfrom = point3(0, 0, 0);
  point3 lookat = point3(0, 0, -1);
  vec3 vup = vec3(0, 1, 0);

  void render(const hittable &world) {
    initialize();

    std::cout << "P3\n" << width << " " << height << "\n255\n";

    for (int j = 0; j < height; j++) {
      std::clog << "Lines remaining: " << (height - j) << " " << std::flush;
      for (int i = 0; i < width; i++) {
        color pixel_color(0, 0, 0);
        for (int sample = 0; sample < samples_per_pixel; sample++) {
          ray r = get_ray(i, j);
          pixel_color += ray_color(r, world, max_depth);
        }
        write_color(std::cout, pixel_samples_scale * pixel_color);
      }
    }

    std::clog << "\rDone                 \n";
  }

private:
  int height;
  point3 center;
  double pixel_samples_scale;
  point3 pixel00_loc;
  vec3 pixel_u_delta;
  vec3 pixel_v_delta;
  vec3 u, v, w;

  void initialize() {

    height = int(width / aspect_ratio);
    height = (height < 1) ? 1 : height;

    pixel_samples_scale = 1.0 / samples_per_pixel;

    center = lookfrom;

    auto theta = degrees_to_radians(vfov);
    auto h = tan(theta / 2);
    auto viewport_height = 2.0 * h;
    auto viewport_width = aspect_ratio * viewport_height;

    w = unit_vector(lookfrom - lookat);
    u = unit_vector(cross(vup, w));
    v = cross(w, u);

    auto focal_length = (lookfrom - lookat).length();

    vec3 viewport_u = viewport_width * u;
    vec3 viewport_v = viewport_height * -v;

    pixel_u_delta = viewport_u / width;
    pixel_v_delta = viewport_v / height;

    auto viewport_upper_left =
        center - focal_length * w - viewport_u / 2 - viewport_v / 2;

    pixel00_loc = viewport_upper_left + 0.5 * (pixel_u_delta + pixel_v_delta);
  };

  ray get_ray(int i, int j) const {
    auto offset = sample_square();
    auto pixel_sample = pixel00_loc + ((i + offset.x()) * pixel_u_delta) +
                        ((j + offset.y()) * pixel_v_delta);

    auto ray_origin = center;
    auto ray_direction = pixel_sample - ray_origin;

    return ray(ray_origin, ray_direction);
  }

  vec3 sample_square() const {
    return vec3(random_double() - 0.5, random_double() - 0.5, 0);
  }

  color ray_color(const ray &r, const hittable &world, int depth) const {
    if (depth <= 0)
      return color(0, 0, 0);

    hit_record rec;
    if (world.hit(r, interval(0.001, infinity), rec)) {
      // DEBUG GUARD: if mat is null, return bright magenta to show the problem
      if (!rec.mat) {
        return color(1.0, 0.0, 1.0); // bright magenta = error marker
      }

      ray scattered;
      color attenuation;
      if (rec.mat->scatter(r, rec, attenuation, scattered)) {
        return attenuation * ray_color(scattered, world, depth - 1);
      }
      return color(0, 0, 0);
    }

    vec3 unit_direction = unit_vector(r.direction());
    auto a = 0.5 * (unit_direction.y() + 1.0);
    return (1.0 - a) * color(1.0, 1.0, 1.0) + a * color(0.0, 0.0, 1.0);
  }
};

#endif