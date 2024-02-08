#pragma once
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
        return math::quat(w, x, y, z);
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
};