#include "bounding_volume.hpp"

#include <array>
#include <optional>
#include <vector>

#include "components.hpp"
#include "src/ecs/entities.hpp"
#include "src/math/math.hpp"

namespace core {

BoundingSphere::BoundingSphere(math::vec3 center, float radius)
    : center(center), radius(radius) {}

std::optional<math::vec3> BoundingSphere::intersect_ray(math::ray ray) const {
  math::vec3 oc = ray.origin - center;

  float a = math::dot(ray.direction, ray.direction);
  float b = 2.0f * math::dot(oc, ray.direction);
  float c = math::dot(oc, oc) - (radius * radius);

  float discriminant = (b * b) - (4 * a * c);
  if (discriminant < 0) {
    return std::nullopt;
  }

  float t1 = (-b + math::sqrt(discriminant)) / (2 * a);
  float t2 = (-b - math::sqrt(discriminant)) / (2 * a);

  if (t1 > t2) {
    std::swap(t1, t2);
  }

  if (t1 >= 0) {
    return ray.origin + (t1 * ray.direction);
  } else if (t2 >= 0) {
    return ray.origin + (t2 * ray.direction);
  }

  return std::nullopt;
}

AABB::AABB(math::vec3 min, math::vec3 max) : bounds({min, max}) {}

float AABB::surface_area() {
  math::vec3 measurements{0.0};

  for (auto i = 0; i < 3; i++) {
    measurements[i] = bounds[1][i] - bounds[0][i];
  }

  float surface_area = 0.0f;

  for (auto i = 0; i < 3; i++) {
    auto ii = (i + 1) % 3;

    surface_area += measurements[i] * measurements[ii] * 2.0f;
  }

  return surface_area;
}

std::optional<math::vec3> AABB::intersect_ray(math::ray ray) const {
  float t_min = (bounds[ray.sign.x].x - ray.origin.x) * ray.inverse_direction.x;
  float t_max =
      (bounds[1 - ray.sign.x].x - ray.origin.x) * ray.inverse_direction.x;

  float ty_min =
      (bounds[ray.sign.y].y - ray.origin.y) * ray.inverse_direction.y;
  float ty_max =
      (bounds[1 - ray.sign.y].y - ray.origin.y) * ray.inverse_direction.y;

  if (t_min > ty_max || ty_min > t_max) {
    return std::nullopt;
  }

  if (ty_min > t_min) {
    t_min = ty_min;
  }

  if (ty_max < t_max) {
    t_max = ty_max;
  }

  float tz_min =
      (bounds[ray.sign.z].z - ray.origin.z) * ray.inverse_direction.z;
  float tz_max =
      (bounds[1 - ray.sign.z].z - ray.origin.z) * ray.inverse_direction.z;

  if (t_min > tz_max || tz_min > t_max) {
    return std::nullopt;
  }

  if (tz_min > t_min) {
    t_min = tz_min;
  }

  if (tz_max < t_max) {
    t_max = tz_max;
  }

  if (t_min > t_max) {
    std::swap(t_min, t_max);
  }

  if (t_min >= 0) {
    return ray.origin + ray.direction * t_min;
  } else if (t_max >= 0) {
    return ray.origin + ray.direction * t_max;
  }

  return std::nullopt;
}

std::optional<math::vec3> OBB::intersect_ray(math::ray ray) const {
  return std::nullopt;
}

math::ray screen_space_slick_to_ray(ecs::World& world, ecs::EntityId camera,
                                    math::ivec2 click_point,
                                    math::ivec2 resolution) {
  auto [view] = world.get_components<View>(camera);

  auto inverse_view = math::inverse(view.projection_matrix * view.view_matrix);

  click_point.y = resolution.y - click_point.y;

  math::vec2 half_resolution = math::vec2(resolution.x, resolution.y) / 2.0f;

  auto near = math::vec4(
      (click_point.x - half_resolution.x) / half_resolution.x,
      (click_point.y - half_resolution.y) / half_resolution.y, 0.0f, 1.0f);
  auto far = math::vec4((click_point.x - half_resolution.x) / half_resolution.x,
                        (click_point.y - half_resolution.y) / half_resolution.y,
                        1.0f, 1.0f);

  near = inverse_view * near;
  far = inverse_view * far;

  math::vec3 origin = near / near.w;
  math::vec3 direction = math::normalize((math::vec3(far) / far.w) - origin);

  return math::ray(origin, direction);
}

TriangularPlane::TriangularPlane(math::vec3 v0, math::vec3 v1, math::vec3 v2)
    : v0(v0), v1(v1), v2(v2) {}

std::optional<math::vec3> TriangularPlane::intersect_ray(math::ray ray) const {
  math::vec3 edge1 = v1 - v0;
  math::vec3 edge2 = v2 - v0;

  math::vec3 h = math::cross(ray.direction, edge2);
  float a = math::dot(edge1, h);

  if (a > -0.00001f && a < 0.00001f) {
    return std::nullopt;
  }

  float f = 1.0f / a;
  math::vec3 s = ray.origin - v0;
  float u = f * math::dot(s, h);

  if (u < 0.0f || u > 1.0f) {
    return std::nullopt;
  }

  math::vec3 q = math::cross(s, edge1);
  float v = f * math::dot(ray.direction, q);

  if (v < 0.0f || u + v > 1.0f) {
    return std::nullopt;
  }

  float t = f * math::dot(edge2, q);

  if (t > 0.00001f) {
    return ray.origin + ray.direction * t;
  }

  return std::nullopt;
}

}  // namespace core