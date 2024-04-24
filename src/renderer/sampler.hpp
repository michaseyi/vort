#pragma once
#include "bind_group_entries.hpp"
#include "src/utils/raii.hpp"
#include "wgpu_context.hpp"

namespace renderer {

class Sampler {
 public:
  Sampler() {
    wgpu::SamplerDescriptor sampler_desc = wgpu::Default;

    auto& context = WgpuContext::get();
    auto device = context.get_device();

    sampler_desc.addressModeU = wgpu::AddressMode::Repeat;
    sampler_desc.addressModeV = wgpu::AddressMode::Repeat;
    sampler_desc.addressModeW = wgpu::AddressMode::Repeat;

    sampler_desc.minFilter = wgpu::FilterMode::Nearest;
    sampler_desc.magFilter = wgpu::FilterMode::Linear;

    sampler_desc.mipmapFilter = wgpu::MipmapFilterMode::Linear;
    sampler_desc.label = "Texture Sampler";

    sampler_ = device.createSampler(sampler_desc);
  }

  operator BindGroupEntries::EntryData() {
    BindGroupEntries::EntryData entry_data;
    entry_data.entry.sampler = *sampler_;
    entry_data.layout_entry.sampler.type = wgpu::SamplerBindingType::Filtering;
    entry_data.layout_entry.visibility =
        wgpu::ShaderStage::Fragment | wgpu::ShaderStage::Compute;
    return entry_data;
  }

 private:
  RaiiWrapper<wgpu::Sampler> sampler_;
};
}  // namespace renderer