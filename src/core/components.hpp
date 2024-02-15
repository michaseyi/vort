#pragma once
#include <memory>

#include "bounding_volume.hpp"
#include "mesh.hpp"
#include "src/math/math.hpp"

#define VECTOR3_COMPONENT(componentName)                              \
    struct componentName {                                            \
        float x;                                                      \
        float y;                                                      \
        float z;                                                      \
        componentName() = default;                                    \
        componentName(float v) : componentName(math::vec3(v)) {       \
        }                                                             \
        componentName(float x, float y, float z) : x(x), y(y), z(z) { \
        }                                                             \
        componentName(math::vec3 v) {                                 \
            x = v.x;                                                  \
            y = v.y;                                                  \
            z = v.z;                                                  \
        }                                                             \
                                                                      \
        operator math::vec3() {                                       \
            return math::vec3(x, y, z);                               \
        }                                                             \
    }

VECTOR3_COMPONENT(Position);
VECTOR3_COMPONENT(Scale);
VECTOR3_COMPONENT(Velocity);

struct Orientation {
    float x;
    float y;
    float z;
    float w;

    Orientation() : Orientation(math::angleAxis(0.0f, math::vec3(1.0f, 0.0f, 0.0f))) {
    }

    Orientation(math::quat v) : x(v.x), y(v.y), z(v.z), w(v.w) {
    }

    operator math::quat() {
        math::quat res;
        res.w = w;
        res.x = x;
        res.y = y;
        res.z = z;
        return res;
    }
};

struct RenderMode {
    enum class Options {
        Surface,
        Edge,
        Vertex,
    };

    uint32_t bits;
};

struct CameraSettings {
    CameraSettings() = default;
    CameraSettings(float fov, float aspectRatio, float near, float far)
        : fov(fov), aspectRatio(aspectRatio), near(near), far(far) {
    }
    float fov;
    float aspectRatio;
    float near;
    float far;
    math::vec3 reference{0.0f};
    float zoom = 0.0f;
    float zoomSensitivity = 1 / 60.0f;
    float rotationSensitivity = 1 / 120.0f;
};

struct View {
    math::mat4 projectionMatrix;
    math::mat4 viewMatrix;
};

struct Interactable {
    Interactable() = default;
    Interactable(BoundingVolume *tBoundingVolume) : boundingVolume(tBoundingVolume) {
    }
    std::unique_ptr<BoundingVolume> boundingVolume;

    bool hovered;
};