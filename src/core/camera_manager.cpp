#include "camera_manager.hpp"

#include <vector>

#include "components.hpp"
#include "data_structs.hpp"
#include "src/ecs/ecs.hpp"
#include "src/math/math.hpp"

namespace core {

CameraManager CameraManager::from_entity_id(ecs::World& world,
                                            ecs::EntityId camera_entity_id) {
  CameraManager camera;

  auto [settings, position, orientation, view, rotation_gizmo] =
      world.get_components<CameraSettings, Position, Orientation, View,
                           RotationGizmo>(camera_entity_id);

  camera.settings = settings;
  camera.position = position;
  camera.orientation = orientation;
  camera.view = view;
  camera.entity_id_ = camera_entity_id;
  camera.rotation_gizmo = rotation_gizmo;

  camera.update_gizmo_data();

  return camera;
}

void CameraManager::save(ecs::World& world) {
  auto [settings, position, orientation, view, rotation_gizmo] =
      world.get_components<CameraSettings, Position, Orientation, View,
                           RotationGizmo>(entity_id_);

  settings = this->settings;
  position = this->position;
  orientation = this->orientation;
  view = this->view;
  rotation_gizmo = this->rotation_gizmo;
}

CameraManager::CameraManager(math::vec3 position, CameraSettings settings)
    : position(position), settings(settings) {
  auto front =
      math::normalize(settings.reference - static_cast<math::vec3>(position));

  orientation = math::normalize(math::quat(kXDirection, front));

  update_view();
}

void CameraManager::update_view() {
  auto front =
      math::normalize(static_cast<math::quat>(orientation) * kXDirection);
  auto up = math::normalize(static_cast<math::quat>(orientation) * kYDirection);

  view.view_matrix =
      math::lookAt(static_cast<math::vec3>(position),
                   static_cast<math::vec3>(position) + front, up);

  view.projection_matrix =
      math::perspectiveZO(math::radians(settings.fov), settings.aspect_ratio,
                          settings.near, settings.far);

  update_gizmo_data();
}

void CameraManager::update_gizmo_data() {
  auto front =
      math::normalize(static_cast<math::quat>(orientation) * kXDirection);
  auto up = math::normalize(static_cast<math::quat>(orientation) * kYDirection);

  math::mat4 gizmo_transform_matrix{1.0f};
  gizmo_transform_matrix =
      math::translate(gizmo_transform_matrix, math::vec3(0.0f, 1.0f, 0.0f));
  gizmo_transform_matrix =
      math::scale(gizmo_transform_matrix, math::vec3(1.0f, -1.0f, 1.0f));
  gizmo_transform_matrix =
      math::translate(gizmo_transform_matrix, math::vec3(0.5f, 0.5f, 0.0f));
  gizmo_transform_matrix =
      math::scale(gizmo_transform_matrix, math::vec3(0.5f, 0.5f, 1.0f));
  gizmo_transform_matrix *= math::lookAt(-front, math::vec3(0.0f), up);

  rotation_gizmo.positive_x_axis =
      gizmo_transform_matrix * math::vec4(1.0f, 0.0f, 0.0f, 1.0f);
  rotation_gizmo.positive_y_axis =
      gizmo_transform_matrix * math::vec4(0.0f, 1.0f, 0.0f, 1.0f);
  rotation_gizmo.positive_z_axis =
      gizmo_transform_matrix * math::vec4(0.0f, 0.0f, 1.0f, 1.0f);
  rotation_gizmo.negative_x_axis =
      gizmo_transform_matrix * math::vec4(-1.0f, 0.0f, 0.0f, 1.0f);
  rotation_gizmo.negative_y_axis =
      gizmo_transform_matrix * math::vec4(0.0f, -1.0f, 0.0f, 1.0f);
  rotation_gizmo.negative_z_axis =
      gizmo_transform_matrix * math::vec4(0.0f, 0.0f, -1.0f, 1.0f);
}

void CameraManager::move(float x_movement, float y_movement, float z_movement) {
  auto up = math::normalize(static_cast<math::quat>(orientation) * kYDirection);
  auto side =
      math::normalize(static_cast<math::quat>(orientation) * kZDirection);
  auto front =
      math::normalize(static_cast<math::quat>(orientation) * kXDirection);

  auto translation_mat = math::translate(
      math::mat4(1.0f), up * y_movement * settings.move_sensitivity);
  translation_mat = math::translate(
      translation_mat, side * x_movement * settings.move_sensitivity);

  position = position - front * z_movement * settings.move_sensitivity;

  if (math::dot(math::normalize(settings.reference - math::vec3(position)),
                front) < 0.0) {
    position = settings.reference - (front * 0.001f);
  }

  position =
      math::vec3(translation_mat * math::vec4(math::vec3(position), 1.0f));
  settings.reference =
      math::vec3(translation_mat * math::vec4(settings.reference, 1.0f));

  update_view();
}

void CameraManager::rotate(float pitch_angle, float yaw_angle) {
  auto yaw_rotation =
      math::angleAxis(yaw_angle * settings.rotation_sensitivity, kYDirection);
  auto pitch_rotation = math::angleAxis(
      pitch_angle * settings.rotation_sensitivity,
      math::normalize(static_cast<math::quat>(orientation) * kZDirection));

  orientation = math::normalize(yaw_rotation * pitch_rotation *
                                static_cast<math::quat>(orientation));

  position =
      settings.reference +
      ((static_cast<math::quat>(orientation) * kXDirection) *
       -math::distance(static_cast<math::vec3>(position), settings.reference));
  update_view();
}

}  // namespace core