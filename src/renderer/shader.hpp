#pragma once
#include "src/utils/raii.hpp"
#include "src/utils/utils.hpp"
#include "wgpu_context.hpp"

namespace renderer {

class Shader {
 public:
  Shader(std::string path) {
    auto shader_code = preprocess_shader(load_shader_code(path));
    wgpu::ShaderModuleWGSLDescriptor shader_module_wgsl_desc = wgpu::Default;

    shader_module_wgsl_desc.code = shader_code.c_str();
    shader_module_wgsl_desc.chain.sType =
        wgpu::SType::ShaderModuleWGSLDescriptor;

    wgpu::ShaderModuleDescriptor shader_module_desc = wgpu::Default;

    shader_module_desc.nextInChain = &shader_module_wgsl_desc.chain;

    auto& context = WgpuContext::get();
    auto device = context.get_device();
    shader_module = device.createShaderModule(shader_module_desc);
  }

  RaiiWrapper<wgpu::ShaderModule> shader_module;
};
}  // namespace renderer