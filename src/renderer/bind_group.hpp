#pragma once
#include "bind_group_entries.hpp"
#include "wgpu_context.hpp"

namespace renderer {
struct BindGroup {
  RaiiWrapper<wgpu::BindGroup> bind_group;
  RaiiWrapper<wgpu::BindGroupLayout> bind_group_layout;

  operator wgpu::BindGroup() { return *bind_group; }

  operator wgpu::BindGroupLayout() { return *bind_group_layout; }

  static BindGroup create(std::string name, BindGroupEntries resources) {
    auto& context = WgpuContext::get();
    wgpu::Device device = context.get_device();

    BindGroup bind_group;

    wgpu::BindGroupLayoutDescriptor bind_group_layout_desc = wgpu::Default;
    bind_group_layout_desc.entryCount = resources.layout_entries.size();
    bind_group_layout_desc.entries = resources.layout_entries.data();
    bind_group_layout_desc.label = name.c_str();

    bind_group.bind_group_layout =
        device.createBindGroupLayout(bind_group_layout_desc);

    wgpu::BindGroupDescriptor bind_group_desc = wgpu::Default;
    bind_group_desc.layout = *bind_group.bind_group_layout;
    bind_group_desc.entryCount = resources.entries.size();
    bind_group_desc.entries = resources.entries.data();
    bind_group_desc.label = name.c_str();

    bind_group.bind_group = device.createBindGroup(bind_group_desc);

    return bind_group;
  }
};
}  // namespace renderer
