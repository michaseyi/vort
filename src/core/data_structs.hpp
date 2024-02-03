#pragma once

#include <array>

#include "src/math/math.hpp"
using namespace math;
// Host-sharable structures

struct AmbientLight {
    AmbientLight() = default;
    AmbientLight(vec3 base_color, float intensity) : base_color(base_color), intensity(intensity) {
    }
    vec3 base_color;  // at byte offset 0
    float intensity;  // at byte offset 12
};
struct PointLight {
    PointLight() = default;
    PointLight(vec3 position, float intensity, vec3 color, vec3 attenuation_coefficients)
        : position(position),
          intensity(intensity),
          color(color),
          attenuation_coefficients(attenuation_coefficients) {
    }
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
    vec3 position;                  // at byte offset 0
    float inner_cone_angle;         // at byte offset 12
    vec3 direction;                 // at byte offset 16
    float outer_cone_angle;         // at byte offset 28
    vec3 color;                     // at byte offset 32
    float intensity;                // at byte offset 44
    vec3 attenuation_coefficients;  // at byte offset 48
    float _pad0;
};
struct CommonUniforms {
    mat4x4 view_projection_matrix;  // at byte offset 0
    float time;                     // at byte offset 64
    float _pad0;
    uvec2 resolution;                            // at byte offset 72
    std::array<AmbientLight, 5> ambient_lights;  // at byte offset 80
    uint32_t ambient_light_count;                // at byte offset 160
    float _pad1[3];
    std::array<PointLight, 5> point_lights;  // at byte offset 176
    uint32_t point_light_count;              // at byte offset 416
    float _pad2[3];
    std::array<DirectionalLight, 5> directional_lights;  // at byte offset 432
    uint32_t directional_light_count;                    // at byte offset 672
    float _pad3[3];
    std::array<SpotLight, 5> spot_lights;  // at byte offset 688
    uint32_t spot_light_count;             // at byte offset 1008
    float _pad4[3];
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