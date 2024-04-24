#pragma once
#include <memory>

#include "material.hpp"
#include "src/core/data_structs.hpp"
#include "src/math/math.hpp"
namespace renderer::materials {

struct PlainMaterialUniform {
  math::vec4 color;
};

struct PlainMaterialProps {
  math::vec3 color;
};

class PlainMaterial : public Material {
 public:
  PlainMaterial(PlainMaterialProps props);

  void update(uint32_t index) override;

  void set_color(math::vec3 color) override;

  ~PlainMaterial();

  uint32_t uniform_offset() override;

  wgpu::ShaderModule shader() override;

  ResourceGroupEntry resource_group_entry() override;

  inline static std::shared_ptr<Uniform> plain_material_uniform;
  inline static std::unique_ptr<Shader> plain_material_shader;

  PlainMaterial(PlainMaterial&&);
  PlainMaterial& operator=(PlainMaterial&&);

 private:
  inline static bool initialized_;
  inline static uint64_t current_index_ = 0;
  inline static uint64_t last_set_index_ = 0;

  inline static bool increment_on_next_ = false;
  ResourceGroup resource_group_;

  bool valid_ = true;
  PlainMaterialProps props_;
};
}  // namespace renderer::materials