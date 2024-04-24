#pragma once

#include <cstdint>

#include "../shader.hpp"
#include "../uniforms.hpp"
#include "../wgpu_context.hpp"
#include "src/math/math.hpp"

namespace renderer::materials {

class Material {
 public:
  virtual void update(uint32_t index) = 0;

  virtual ResourceGroupEntry resource_group_entry() = 0;

  virtual wgpu::ShaderModule shader() = 0;

  virtual uint32_t uniform_offset() = 0;

  virtual void set_color(math::vec3 color);
  virtual void set_reflectivity(float reflectivity);
  virtual void set_roughness(float roughness);
  virtual void set_shininess(float shininess);

  virtual void set_texture();

  virtual ~Material();
};

}  // namespace renderer::materials