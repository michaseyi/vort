#pragma once
#include <array>
#include "bind_group.hpp"
#include "wgpu_context.hpp"

namespace renderer {
struct Pipeline {
  std::array<wgpu::BindGroup, 4> bind_groups = {nullptr, nullptr, nullptr,
                                                nullptr};
  RaiiWrapper<wgpu::PipelineLayout> pipeline_layout;

  template <typename... T>
    requires(sizeof...(T) <= 4 && (std::is_same_v<T, BindGroup> && ...))
  static Pipeline create(std::string name, std::tuple<T&...> bind_groups) {
    auto& context = WgpuContext::get();
    wgpu::Device device = context.get_device();

    Pipeline pipeline;

    WGPUBindGroupLayout bind_group_layouts[4] = {nullptr, nullptr, nullptr,
                                                 nullptr};

    ([&]<std::size_t... I>(std::index_sequence<I...>) -> void {
      ((pipeline.bind_groups[I] = std::get<I>(bind_groups).bind_group), ...);
      ((bind_group_layouts[I] = *std::get<I>(bind_groups).bind_group_layout),
       ...);
    }(std::make_index_sequence<sizeof...(T)>{}));

    wgpu::PipelineLayoutDescriptor pipeline_layout_desc = wgpu::Default;
    pipeline_layout_desc.bindGroupLayoutCount = sizeof...(T);
    pipeline_layout_desc.label = name.c_str();
    pipeline_layout_desc.bindGroupLayouts = bind_group_layouts;
    pipeline.pipeline_layout =
        device.createPipelineLayout(pipeline_layout_desc);

    return pipeline;
  }
};
}  // namespace renderer
