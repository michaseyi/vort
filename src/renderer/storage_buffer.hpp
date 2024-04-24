#pragma once

#include "bind_group_entries.hpp"
#include "wgpu_context.hpp"

namespace renderer {
template <typename T>
  requires(sizeof(T) % 16 == 0)
struct StorageBuffer {
  StorageBuffer(uint64_t size, bool read_only = true)
      : size_(size), read_only_(read_only) {
    auto& context = WgpuContext::get();
    wgpu::Device device = context.get_device();

    wgpu::BufferDescriptor buffer_desc = wgpu::Default;
    buffer_desc.mappedAtCreation = false;
    buffer_desc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Storage;

    buffer_desc.size = sizeof(T) * size_;

    buffer_desc.label = "StorageBuffer";
    storage_buffer_ = device.createBuffer(buffer_desc);
  }

  operator BindGroupEntries::EntryData() {
    BindGroupEntries::EntryData entry_data;
    entry_data.layout_entry.buffer.hasDynamicOffset = false;
    entry_data.layout_entry.buffer.type =
        read_only_ ? wgpu::BufferBindingType::ReadOnlyStorage
                   : wgpu::BufferBindingType::Storage;
    entry_data.layout_entry.visibility =
        wgpu::ShaderStage::Compute | wgpu::ShaderStage::Fragment;

    entry_data.entry.buffer = *storage_buffer_;
    entry_data.entry.offset = 0;
    entry_data.entry.size = sizeof(T) * size_;

    return entry_data;
  }

  void set(uint64_t index, T& data) {
    assert(index < size_);
    assert(*storage_buffer_);
    auto& context = WgpuContext::get();
    wgpu::Queue queue = context.get_queue();
    queue.writeBuffer(*storage_buffer_, sizeof(T) * index, &data, sizeof(T));
  }

  void set(uint64_t index, T&& data) {
    T temp = data;
    set(index, temp);
  }

  void set(uint64_t offset, uint64_t size, T* data) {
    assert(offset + size <= size_);
    assert(*storage_buffer_);
    auto& context = WgpuContext::get();
    wgpu::Queue queue = context.get_queue();
    queue.writeBuffer(*storage_buffer_, sizeof(T) * offset, data,
                      sizeof(T) * size);
  }

 private:
  RaiiWrapper<wgpu::Buffer> storage_buffer_;
  uint64_t size_;
  bool read_only_;
};

}  // namespace renderer
