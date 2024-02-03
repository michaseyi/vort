#pragma once
#include "mesh.hpp"
#include "src/math/math.hpp"

#define VECTOR3_COMPONENT(componentName)                              \
    struct componentName {                                            \
        float x;                                                      \
        float y;                                                      \
        float z;                                                      \
        componentName() = default;                                    \
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

#define VECTOR3_COMPONENT(componentName)                              \
    struct componentName {                                            \
        float x;                                                      \
        float y;                                                      \
        float z;                                                      \
        componentName() = default;                                    \
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

    Orientation(math::quat v) : x(v.x), y(v.y), z(v.z), w(v.w) {
    }

    Orientation(float w, float x, float y, float z) : x(x), y(y), z(z), w(w) {
    }

    operator math::quat() {
        return math::quat(w, x, y, z);
    }
    Orientation() = default;
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