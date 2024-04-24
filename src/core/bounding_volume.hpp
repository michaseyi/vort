#pragma once
#include <array>
#include <limits>
#include <optional>
#include <vector>

#include "src/ecs/entities.hpp"
#include "src/math/math.hpp"

namespace core {

struct BoundingVolume {
  virtual std::optional<math::vec3> intersect_ray(math::ray ray) const = 0;

  virtual ~BoundingVolume() = default;
};

struct BoundingSphere : public BoundingVolume {
  BoundingSphere(math::vec3 center, float radius);

  std::optional<math::vec3> intersect_ray(math::ray ray) const override;

  math::vec3 center;
  float radius;
};

struct AABB : public BoundingVolume {
  AABB(math::vec3 min = math::vec3{std::numeric_limits<float>::max()},
       math::vec3 max = math::vec3{std::numeric_limits<float>::min()});

  float surface_area();

  std::optional<math::vec3> intersect_ray(math::ray ray) const override;
  std::array<math::vec3, 2> bounds;
};

struct TriangularPlane : public BoundingVolume {
  TriangularPlane(math::vec3 v0, math::vec3 v1, math::vec3 v2);

  std::optional<math::vec3> intersect_ray(math::ray ray) const override;

  math::vec3 v0, v1, v2;
};
struct OBB : public BoundingVolume {
  math::vec3 center;
  math::vec3 half_extents;
  math::quat orientation;

  std::optional<math::vec3> intersect_ray(math::ray ray) const override;
};

math::ray screen_space_slick_to_ray(ecs::World& world, ecs::EntityId camera,
                                    math::ivec2 click_point,
                                    math::ivec2 resolution);

}  // namespace core