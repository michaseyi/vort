#pragma once

#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

namespace math {

    using namespace glm;

    struct ray {
        vec3 origin;
        vec3 direction;
        vec3 inverseDirection;
        uvec3 sign;

        ray(vec3 origin, vec3 direction) : origin(origin), direction(direction), inverseDirection(1.0f / direction) {
            sign.x = direction.x < 0;
            sign.y = direction.y < 0;
            sign.z = direction.z < 0;
        }
    };
}  // namespace math

inline std::ostream& operator<<(std::ostream& in, glm::vec3 vec) {
    in << "vec3{";
    in << vec.x << ", ";
    in << vec.y << ", ";
    in << vec.z;
    in << "}";
    return in;
}