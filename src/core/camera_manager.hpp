#pragma once
#include <vector>

#include "components.hpp"
#include "data_structs.hpp"
#include "src/ecs/ecs.hpp"
#include "src/math/math.hpp"

namespace core {

struct CameraManager {
  math::vec3 position;
  math::quat orientation;
  View view;
  CameraSettings settings;
  RotationGizmo rotation_gizmo;

  inline const static math::vec3 kXDirection = math::vec3(1.0f, 0.0f, 0.0f);
  inline const static math::vec3 kYDirection = math::vec3(0.0f, 1.0f, 0.0f);
  inline const static math::vec3 kZDirection = math::vec3(0.0f, 0.0f, 1.0f);

  CameraManager() = default;

  static CameraManager from_entity_id(ecs::World& world,
                                      ecs::EntityId camera_entity_id);

  void save(ecs::World& world);

  CameraManager(math::vec3 position, CameraSettings settings);

  void update_view();

  void update_gizmo_data();

  void move(float x_movement, float y_ovement, float z_movement);

  void rotate(float pitch_angle, float yaw_angle);

 private:
  ecs::EntityId entity_id_ = ecs::Entities::kRootEntityId;
};
}  // namespace core
