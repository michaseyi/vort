#pragma once
#include "src/utils/raii.hpp"
#include "src/utils/utils.hpp"
#include "wgpu_context.hpp"

class Shader {
public:
    Shader(std::string path) {
        auto shaderCode = loadShaderCode(path);

        wgpu::ShaderModuleWGSLDescriptor shaderModuleWGSLDesc = wgpu::Default;

        shaderModuleWGSLDesc.code = shaderCode.c_str();
        shaderModuleWGSLDesc.chain.sType = wgpu::SType::ShaderModuleWGSLDescriptor;

        wgpu::ShaderModuleDescriptor shaderModuleDesc = wgpu::Default;

        shaderModuleDesc.nextInChain = &shaderModuleWGSLDesc.chain;

        auto &context = WGPUContext::getContext();
        auto device = context.getDevice();
        shaderModule = device.createShaderModule(shaderModuleDesc);
    }

private:
    RAIIWrapper<wgpu::ShaderModule> shaderModule;
};