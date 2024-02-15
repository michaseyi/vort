#include "bounding_volume.hpp"

#include <array>
#include <optional>
#include <vector>

#include "components.hpp"
#include "src/ecs/entities.hpp"
#include "src/math/math.hpp"

BoundingSphere::BoundingSphere(math::vec3 center, float radius) : center(center), radius(radius) {
}

std::optional<math::vec3> BoundingSphere::intersectRay(math::ray tRay) const {
    math::vec3 oc = tRay.origin - center;

    float a = math::dot(tRay.direction, tRay.direction);
    float b = 2.0f * math::dot(oc, tRay.direction);
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
        return tRay.origin + (t1 * tRay.direction);
    } else if (t2 >= 0) {
        return tRay.origin + (t2 * tRay.direction);
    }

    return std::nullopt;
}

AABB::AABB(math::vec3 minPoint, math::vec3 maxPoint) : bounds({minPoint, maxPoint}) {
}

std::optional<math::vec3> AABB::intersectRay(math::ray tRay) const {
    float tMin = (bounds[tRay.sign.x].x - tRay.origin.x) * tRay.inverseDirection.x;
    float tMax = (bounds[1 - tRay.sign.x].x - tRay.origin.x) * tRay.inverseDirection.x;

    float tyMin = (bounds[tRay.sign.y].y - tRay.origin.y) * tRay.inverseDirection.y;
    float tyMax = (bounds[1 - tRay.sign.y].y - tRay.origin.y) * tRay.inverseDirection.y;

    if (tMin > tyMax || tyMin > tMax) {
        return std::nullopt;
    }

    if (tyMin > tMin) {
        tMin = tyMin;
    }

    if (tyMax < tMax) {
        tMax = tyMax;
    }

    float tzMin = (bounds[tRay.sign.z].z - tRay.origin.z) * tRay.inverseDirection.z;
    float tzMax = (bounds[1 - tRay.sign.z].z - tRay.origin.z) * tRay.inverseDirection.z;

    if (tMin > tzMax || tzMin > tMax) {
        return std::nullopt;
    }

    if (tzMin > tMin) {
        tMin = tzMin;
    }

    if (tzMax < tMax) {
        tMax = tzMax;
    }

    if (tMin > tMax) {
        std::swap(tMin, tMax);
    }

    if (tMin >= 0) {
        return tRay.origin + tRay.direction * tMin;
    } else if (tMax >= 0) {
        return tRay.origin + tRay.direction * tMax;
    }

    return std::nullopt;
}

std::optional<math::vec3> OBB::intersectRay(math::ray tRay) const {
    return std::nullopt;
}

math::ray windowClickToRay(World& tWorld, EntityID tCamera, math::ivec2 tClickPoint, math::ivec2 tResolution) {
    auto [view] = tWorld.getComponents<View>(tCamera);

    auto inverseView = math::inverse(view.projectionMatrix * view.viewMatrix);

    tClickPoint.y = tResolution.y - tClickPoint.y;

    math::vec2 halfResolution = math::vec2(tResolution.x, tResolution.y) / 2.0f;

    auto near = math::vec4((tClickPoint.x - halfResolution.x) / halfResolution.x,
                           (tClickPoint.y - halfResolution.y) / halfResolution.y, 0.0f, 1.0f);
    auto far = math::vec4((tClickPoint.x - halfResolution.x) / halfResolution.x,
                          (tClickPoint.y - halfResolution.y) / halfResolution.y, 1.0f, 1.0f);

    near = inverseView * near;
    far = inverseView * far;

    math::vec3 origin = near / near.w;
    math::vec3 direction = math::normalize((math::vec3(far) / far.w) - origin);

    return math::ray(origin, direction);
}