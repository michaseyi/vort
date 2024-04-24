#pragma once

#include <array>

#include "src/math/math.hpp"
using namespace math;
// Host-sharable structures
namespace core {

struct AmbientLight {
  AmbientLight() = default;
  AmbientLight(vec3 base_color, float intensity)
      : base_color(base_color), intensity(intensity) {}
  vec3 base_color;  // at byte offset 0
  float intensity;  // at byte offset 12
};
struct PointLight {
  PointLight() = default;
  PointLight(vec3 position, float intensity, vec3 color,
             vec3 attenuation_coefficients)
      : position(position),
        intensity(intensity),
        color(color),
        attenuation_coefficients(attenuation_coefficients) {}
  vec3 position;    // at byte offset 0
  float intensity;  // at byte offset 12
  vec3 color;       // at byte offset 16
  float _pad0;
  vec3 attenuation_coefficients;  // at byte offset 32
  float _pad1;
};

struct DirectionalLight {
  vec3 direction;   // at byte offset 0
  float intensity;  // at byte offset 12
  vec3 color;       // at byte offset 16
  float _pad0;
  vec3 attenuation_coefficients;  // at byte offset 32
  float _pad1;
};
struct SpotLight {
  SpotLight() = default;
  SpotLight(vec3 position, float inner_cone_angle, float outer_cone_angle,
            vec3 direction, vec3 color, float intensity,
            vec3 attenuation_coefficients)
      : position(position),
        inner_cone_angle(inner_cone_angle),
        outer_cone_angle(outer_cone_angle),
        direction(direction),
        color(color),
        intensity(intensity),
        attenuation_coefficients(attenuation_coefficients) {}
  vec3 position;                  // at byte offset 0
  float inner_cone_angle;         // at byte offset 12
  vec3 direction;                 // at byte offset 16
  float outer_cone_angle;         // at byte offset 28
  vec3 color;                     // at byte offset 32
  float intensity;                // at byte offset 44
  vec3 attenuation_coefficients;  // at byte offset 48
  float _pad0;
};

struct BasicMaterialUniforms {
  vec3 base_color;      // at byte offset 0
  float alpha;          // at byte offset 12
  vec3 emissive_color;  // at byte offset 16
  float specular;       // at byte offset 28
  float shininess;      // at byte offset 32
  float _pad0[3];
};
struct ModelUniforms {
  mat4x4 model_matrix;   // at byte offset 0
  mat3x4 normal_matrix;  // at byte offset 64
};

struct Camera {
  vec3 position;  // at byte offset 0
  float _pad0;
  mat4x4 view_projection_matrix;  // at byte offset 16
  float fov;                      // at byte offset 80
  float aspect;                   // at byte offset 84
  float near;                     // at byte offset 88
  float far;                      // at byte offset 92
  float zoom;                     // at byte offset 96
  float _pad1[3];
};

struct CommonUniforms {
  Camera camera;  // at byte offset 0
  float time;     // at byte offset 112
  float _pad0;
  uvec2 resolution;                            // at byte offset 120
  std::array<AmbientLight, 5> ambient_lights;  // at byte offset 128
  uint32_t ambient_light_count;                // at byte offset 208
  float _pad1[3];
  std::array<PointLight, 5> point_lights;  // at byte offset 224
  uint32_t point_light_count;              // at byte offset 464
  float _pad2[3];
  std::array<DirectionalLight, 5> directional_lights;  // at byte offset 480
  uint32_t directional_light_count;                    // at byte offset 720
  float _pad3[3];
  std::array<SpotLight, 5> spot_lights;  // at byte offset 736
  uint32_t spot_light_count;             // at byte offset 1056
  float _pad4[3];
};

}  // namespace core