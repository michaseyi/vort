#pragma once
#include <array>
#include <optional>
#include <vector>

#include "src/ecs/entities.hpp"
#include "src/math/math.hpp"

struct BoundingVolume {
    virtual std::optional<math::vec3> intersectRay(math::ray tRay) const = 0;

    virtual ~BoundingVolume() = default;
};

struct BoundingSphere : public BoundingVolume {
    BoundingSphere(math::vec3 center, float radius);

    std::optional<math::vec3> intersectRay(math::ray tRay) const override;

    math::vec3 center;
    float radius;
};

struct AABB : public BoundingVolume {
    AABB(math::vec3 minPoint, math::vec3 maxPoint);

    std::optional<math::vec3> intersectRay(math::ray tRay) const override;
    std::array<math::vec3, 2> bounds;
};

struct OBB : public BoundingVolume {
    math::vec3 center;
    math::vec3 halfExtents;
    math::quat orientation;

    std::optional<math::vec3> intersectRay(math::ray tRay) const override;
};

math::ray windowClickToRay(World& tWorld, EntityID tCamera, math::ivec2 tClickPoint, math::ivec2 tResolution);