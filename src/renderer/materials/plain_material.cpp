#include "plain_material.hpp"

#include "../uniforms.hpp"
namespace renderer::materials {

PlainMaterial::PlainMaterial(PlainMaterialProps props) {
  if (!initialized_) {
    plain_material_uniform.reset(
        new Uniform{sizeof(PlainMaterialUniform), true});
    plain_material_shader.reset(new Shader("plain_material.wgsl"));
    initialized_ = true;
  }
  resource_group_.set_binding_count(1);
  resource_group_.bind_resource(0, plain_material_uniform);
  props_ = props;
  plain_material_uniform->increment();
}

wgpu::ShaderModule PlainMaterial::shader() {
  return *plain_material_shader->shader_module;
}

uint32_t PlainMaterial::uniform_offset() {
  return current_index_ * plain_material_uniform->stride();
}

ResourceGroupEntry PlainMaterial::resource_group_entry() {
  return resource_group_.resource_group_entry();
}
void PlainMaterial::set_color(math::vec3 color) {
  props_.color = color;
}

void PlainMaterial::update(uint32_t index) {
  if (increment_on_next_) {
    current_index_++;
  }
  if (index < last_set_index_) {
    current_index_ = 0;
  }
  last_set_index_ = index + 1;
  increment_on_next_ = true;

  PlainMaterialUniform uniform{};
  uniform.color = math::vec4(props_.color, 1.0f);
  plain_material_uniform->set(current_index_, 0, sizeof(uniform), &uniform);
}

PlainMaterial::~PlainMaterial() {
  plain_material_uniform->decrement();
}
}  // namespace renderer::materials