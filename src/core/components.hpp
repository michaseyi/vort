#pragma once
#include <memory>

#include "bounding_volume.hpp"
#include "mesh.hpp"
#include "src/math/math.hpp"

#define VECTOR3_COMPONENT(component_name)                           \
  struct component_name {                                           \
    float x;                                                        \
    float y;                                                        \
    float z;                                                        \
    component_name() = default;                                     \
    component_name(float v) : component_name(math::vec3(v)) {}      \
    component_name(float x, float y, float z) : x(x), y(y), z(z) {} \
    component_name(math::vec3 v) {                                  \
      x = v.x;                                                      \
      y = v.y;                                                      \
      z = v.z;                                                      \
    }                                                               \
                                                                    \
    operator math::vec3() {                                         \
      return math::vec3(x, y, z);                                   \
    }                                                               \
  }

namespace core {

VECTOR3_COMPONENT(Position);
VECTOR3_COMPONENT(Scale);
VECTOR3_COMPONENT(Velocity);

struct Orientation {
  float x;
  float y;
  float z;
  float w;

  Orientation()
      : Orientation(math::angleAxis(0.0f, math::vec3(1.0f, 0.0f, 0.0f))) {}

  Orientation(math::quat v) : x(v.x), y(v.y), z(v.z), w(v.w) {}

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
  CameraSettings(float fov, float aspect_ratio, float near, float far)
      : fov(fov), aspect_ratio(aspect_ratio), near(near), far(far) {}
  float fov;
  float aspect_ratio;
  float near;
  float far;
  math::vec3 reference{0.0f};
  float move_sensitivity = 1 / 120.0f;
  float rotation_sensitivity = 1 / 120.0f;
};

struct View {
  math::mat4 projection_matrix;
  math::mat4 view_matrix;
};

struct Interactable {
  Interactable() = default;
  Interactable(BoundingVolume* bounding_volume)
      : bounding_volume(bounding_volume) {}
  std::unique_ptr<BoundingVolume> bounding_volume;
  bool hovered;
};

struct RotationGizmo {
  math::vec3 positive_x_axis;
  math::vec3 positive_y_axis;
  math::vec3 positive_z_axis;
  math::vec3 negative_x_axis;
  math::vec3 negative_y_axis;
  math::vec3 negative_z_axis;
};

}  // namespace core