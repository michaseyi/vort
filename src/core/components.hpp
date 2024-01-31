#pragma once
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

VECTOR3_COMPONENT(Position);
VECTOR3_COMPONENT(Scale);
VECTOR3_COMPONENT(Velocity);
