#pragma once

#include "bind_group_entries.hpp"
#include "wgpu_context.hpp"

namespace renderer {
template <typename T>
  requires(sizeof(T) % 16 == 0)
struct UniformBuffer {
  UniformBuffer() {
    auto& context = WgpuContext::get();
    wgpu::Device device = context.get_device();

    wgpu::BufferDescriptor buffer_desc = wgpu::Default;
    buffer_desc.mappedAtCreation = false;
    buffer_desc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform;
    buffer_desc.size = sizeof(T);

    buffer_desc.label = "UniformBuffer";
    storage_buffer_ = device.createBuffer(buffer_desc);
  }

  void set(T& data) {
    auto& context = WgpuContext::get();
    wgpu::Queue queue = context.get_queue();
    assert(*storage_buffer_);
    queue.writeBuffer(*storage_buffer_, 0, &data, sizeof(T));
  }

  void set(T&& data) {
    T temp = data;
    set(temp);
  }

  operator BindGroupEntries::EntryData() {
    BindGroupEntries::EntryData entry_data;
    entry_data.layout_entry.buffer.hasDynamicOffset = false;
    entry_data.layout_entry.buffer.type = wgpu::BufferBindingType::Uniform;
    entry_data.layout_entry.visibility = wgpu::ShaderStage::Compute |
                                         wgpu::ShaderStage::Fragment |
                                         wgpu::ShaderStage::Vertex;

    entry_data.entry.buffer = *storage_buffer_;
    entry_data.entry.offset = 0;
    entry_data.entry.size = sizeof(T);

    return entry_data;
  }

 private:
  RaiiWrapper<wgpu::Buffer> storage_buffer_;
};
}  // namespace renderer