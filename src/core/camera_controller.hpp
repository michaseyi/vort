#pragma once
#include "src/math/math.hpp"


class CameraController {
public:
    math::vec3 position;
    math::quat orientation;

    inline static math::vec3 xDirection = math::vec3(1.0f, 0.0f, 0.0f);
    inline static math::vec3 yDirection = math::vec3(0.0f, 1.0f, 0.0f);

    View shout() {
        auto front = orientation * xDirection;
        auto up = orientation * yDirection;

        auto viewMatrix = math::translate(math::mat4(1.0f), -position) * math::lookAt(math::vec3(0.0f), front, up);

        View view;
        return view;
    }
};

struct View {
    math::mat4 projectionMatrix;
    math::mat4 viewMatrix;
};