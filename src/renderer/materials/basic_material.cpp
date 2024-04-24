#include "basic_material.hpp"

#include "../uniforms.hpp"

namespace renderer::materials {

BasicMaterial::BasicMaterial(BasicMaterialProps props) {
  if (!initialized_) {
    basic_material_uniform.reset(
        new Uniform{sizeof(core::BasicMaterialUniforms), true});
    basic_material_shader.reset(new Shader("basic_material.wgsl"));
    initialized_ = true;
  }
  resource_group_.set_binding_count(1);
  resource_group_.bind_resource(0, basic_material_uniform);
  props_ = props;
  basic_material_uniform->increment();
}

wgpu::ShaderModule BasicMaterial::shader() {
  return *basic_material_shader->shader_module;
}

uint32_t BasicMaterial::uniform_offset() {
  return current_index_ * basic_material_uniform->stride();
}

ResourceGroupEntry BasicMaterial::resource_group_entry() {
  return resource_group_.resource_group_entry();
}
void BasicMaterial::set_color(math::vec3 color) {
  props_.color = color;
}

void BasicMaterial::update(uint32_t index) {
  if (increment_on_next_) {
    current_index_++;
  }
  if (index < last_set_index_) {
    current_index_ = 0;
  }

  increment_on_next_ = true;

  last_set_index_ = index + 1;
  core::BasicMaterialUniforms uniform{};

  uniform.base_color = props_.color;
  uniform.alpha = 1.0f;
  basic_material_uniform->set(current_index_, 0, sizeof(uniform), &uniform);
}

BasicMaterial::~BasicMaterial() {
  basic_material_uniform->decrement();
}
}  // namespace renderer::materials