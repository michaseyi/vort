#pragma once
#include <vector>

#include "components.hpp"
#include "data_structs.hpp"
#include "src/ecs/ecs.hpp"
#include "src/math/math.hpp"

inline void printMat(math::mat4 mat) {
    for (uint32_t i = 0; i < 4; i++) {
        std::cout << "[";
        for (uint32_t j = 0; j < 4; j++) {
            std::cout << mat[i][j] << " ";
        }
        std::cout << "]";
    }
    std::cout << std::endl;
}

inline void printVec(math::vec3 vec) {
    std::cout << "[";
    for (uint32_t i = 0; i < 3; i++) {
        std::cout << vec[i] << " ";
    }
    std::cout << "]";
    std::cout << std::endl;
}
class CameraController {
public:
    EntityID cameraID = 0;

    EntityID attachedLight = 0;

    inline static math::vec3 xDirection = math::vec3(1.0f, 0.0f, 0.0f);
    inline static math::vec3 yDirection = math::vec3(0.0f, 1.0f, 0.0f);
    inline static math::vec3 zDirection = math::vec3(0.0f, 0.0f, 1.0f);
    CameraController& attachLight(EntityID tLightID) {
        attachedLight = tLightID;
        return *this;
    }

    void updateLight(World& tWorld, math::vec3 tPosition) {
        if (attachedLight == 0) {
            return;
        }

        auto [pointLight] = tWorld.getComponents<PointLight>(attachedLight);
        pointLight.position = tPosition;
    }

    CameraController& attachCamera(EntityID tCameraID) {
        cameraID = tCameraID;

        return *this;
    }

    CameraController& detach() {
        cameraID = 0;
        return *this;
    }

    CameraController& init(World& tWorld) {
        auto [settings, position, orientation, view] =
            tWorld.getComponents<CameraSettings, Position, Orientation, View>(cameraID);

        auto front = math::normalize(settings.reference - static_cast<math::vec3>(position));

        // An orbit camera controller uses the camera reference so we have to compute the orientation based on the position and
        // the reference vector
        // updating the orientation
        // TODO: This should be reactive
        orientation = math::normalize(math::quat(xDirection, front));

        updateView(tWorld);
        return *this;
    }

    void rotate(World& tWorld, float pitchAngle, float yawAngle) {
        auto [orientation, position, settings] = tWorld.getComponents<Orientation, Position, CameraSettings>(cameraID);

        auto yawRot = math::angleAxis(yawAngle * settings.rotationSensitivity, yDirection);
        auto pitchRot = math::angleAxis(pitchAngle * settings.rotationSensitivity,
                                        math::normalize(static_cast<math::quat>(orientation) * zDirection));

        orientation = math::normalize(yawRot * pitchRot * static_cast<math::quat>(orientation));

        position = settings.reference + ((static_cast<math::quat>(orientation) * xDirection) *
                                         -math::distance(static_cast<math::vec3>(position), settings.reference));
        updateView(tWorld);
    }

    void move(World& tWorld, float xMovement, float yMovement, float zMovement) {
        auto [settings, position, orientation] = tWorld.getComponents<CameraSettings, Position, Orientation>(cameraID);

        auto up = math::normalize(static_cast<math::quat>(orientation) * yDirection);
        auto side = math::normalize(static_cast<math::quat>(orientation) * zDirection);
        auto front = math::normalize(static_cast<math::quat>(orientation) * xDirection);

        auto translationMat = math::translate(math::mat4(1.0f), up * yMovement * settings.moveSensitivity);
        translationMat = math::translate(translationMat, side * xMovement * settings.moveSensitivity);

        position = math::vec3(position) - front * zMovement * settings.moveSensitivity;

        if (math::dot(math::normalize(settings.reference - math::vec3(position)), front) < 0.0) {
            position = settings.reference - (front * 0.001f);
        }

        position = math::vec3(translationMat * math::vec4(math::vec3(position), 1.0f));
        settings.reference = math::vec3(translationMat * math::vec4(settings.reference, 1.0f));

        updateView(tWorld);
    }

    void updateView(World& tWorld) {
        auto [settings, position, orientation, view] =
            tWorld.getComponents<CameraSettings, Position, Orientation, View>(cameraID);

        auto [rotationGizmo] = tWorld.getGlobal<RotationGizmo>();

        auto front = math::normalize(static_cast<math::quat>(orientation) * xDirection);
        auto up = math::normalize(static_cast<math::quat>(orientation) * yDirection);

        // TODO: This should be reactive
        view.viewMatrix = math::lookAt(static_cast<math::vec3>(position), static_cast<math::vec3>(position) + front, up);

        // TODO: This should be reactive
        view.projectionMatrix =
            math::perspectiveZO(math::radians(settings.fov), settings.aspectRatio, settings.near, settings.far);

        math::mat4 gizmoTransformMatrix{1.0f};
        gizmoTransformMatrix = math::translate(gizmoTransformMatrix, math::vec3(0.0f, 1.0f, 0.0f));
        gizmoTransformMatrix = math::scale(gizmoTransformMatrix, math::vec3(1.0f, -1.0f, 1.0f));
        gizmoTransformMatrix = math::translate(gizmoTransformMatrix, math::vec3(0.5f, 0.5f, 0.0f));
        gizmoTransformMatrix = math::scale(gizmoTransformMatrix, math::vec3(0.5f, 0.5f, 1.0f));
        gizmoTransformMatrix *= math::lookAt(-front, math::vec3(0.0f), up);

        rotationGizmo.positiveXAxis = gizmoTransformMatrix * math::vec4(1.0f, 0.0f, 0.0f, 1.0f);
        rotationGizmo.positiveYAxis = gizmoTransformMatrix * math::vec4(0.0f, 1.0f, 0.0f, 1.0f);
        rotationGizmo.positiveZAxis = gizmoTransformMatrix * math::vec4(0.0f, 0.0f, 1.0f, 1.0f);
        rotationGizmo.negativeXAxis = gizmoTransformMatrix * math::vec4(-1.0f, 0.0f, 0.0f, 1.0f);
        rotationGizmo.negativeYAxis = gizmoTransformMatrix * math::vec4(0.0f, -1.0f, 0.0f, 1.0f);
        rotationGizmo.negativeZAxis = gizmoTransformMatrix * math::vec4(0.0f, 0.0f, -1.0f, 1.0f);

        updateLight(tWorld, position);
    }
};
