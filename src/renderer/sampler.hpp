#pragma once
#include "resource.hpp"
#include "src/utils/raii.hpp"
#include "wgpu_context.hpp"

class Sampler : public Resource {
public:
    Sampler() {
        wgpu::SamplerDescriptor samplerDesc = wgpu::Default;

        auto &context = WGPUContext::getContext();
        auto device = context.getDevice();

        samplerDesc.addressModeU = wgpu::AddressMode::Repeat;
        samplerDesc.addressModeV = wgpu::AddressMode::Repeat;
        samplerDesc.addressModeW = wgpu::AddressMode::Repeat;

        samplerDesc.minFilter = wgpu::FilterMode::Nearest;
        samplerDesc.magFilter = wgpu::FilterMode::Linear;

        samplerDesc.mipmapFilter = wgpu::MipmapFilterMode::Linear;

        WGPUSampler = device.createSampler(samplerDesc);
    }

    ResourceEntry resourceEntry() const override {
        ResourceEntry entry{};
        entry.bindGroupLayoutEntry.sampler.type = wgpu::SamplerBindingType::Comparison;
        entry.bindGroupLayoutEntry.visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;

        entry.bindGroupEntry.sampler = *WGPUSampler;
        return entry;
    }

private:
    RAIIWrapper<wgpu::Sampler> WGPUSampler;
};