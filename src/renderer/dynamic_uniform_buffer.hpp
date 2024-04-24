#pragma once

#include "bind_group_entries.hpp"
#include "wgpu_context.hpp"
namespace renderer {
template <typename T>
  requires(sizeof(T) % 16 == 0)
struct DynamicUniformBuffer {
  inline static constexpr uint32_t kOffsetAlignment = 256;
  inline static const uint32_t kStride =
      static_cast<uint32_t>(std::ceil(static_cast<double>(sizeof(T)) /
                                      static_cast<double>(kOffsetAlignment))) *
      kOffsetAlignment;

  DynamicUniformBuffer() { resize_buffer(1); }

  operator BindGroupEntries::EntryData() {
    BindGroupEntries::EntryData entry_data;
    entry_data.layout_entry.buffer.hasDynamicOffset = true;
    entry_data.layout_entry.buffer.type = wgpu::BufferBindingType::Uniform;
    entry_data.layout_entry.visibility = wgpu::ShaderStage::Compute |
                                         wgpu::ShaderStage::Fragment |
                                         wgpu::ShaderStage::Vertex;

    entry_data.entry.buffer = *storage_buffer_;
    entry_data.entry.offset = 0;
    entry_data.entry.size = kStride * actual_buffer_size_;
    return entry_data;
  }

  void set(T& data) {
    assert(active_index_.has_value());
    assert(*storage_buffer_);
    auto& context = WgpuContext::get();
    wgpu::Queue queue = context.get_queue();
    queue.writeBuffer(*storage_buffer_, kStride * active_index_.value(), &data,
                      sizeof(T));
  }

  void set(T&& data) {
    T temp = data;
    set(temp);
  }

  void set_active_index(uint32_t index) {
    assert(index < size_);
    active_index_ = index;
  };

  void increment_active_index() {
    assert(active_index_.has_value());

    assert(active_index_.value() < size_ - 1);
    active_index_ = active_index_.value() + 1;
  };

  void decrement_active_index() {
    assert(active_index_.has_value());

    assert(active_index_.value() > 0);
    active_index_ = active_index_.value() - 1;
  };

  void push() {
    if (size_ == actual_buffer_size_) {
      resize_buffer(actual_buffer_size_ == 0 ? 1 : actual_buffer_size_ * 2);
    }
    size_++;

    if (!active_index_.has_value()) {
      active_index_ = 0;
    }
  };

  void pop() {
    assert(size_ > 0);
    size_--;
    if (size_ < actual_buffer_size_ / 2 && actual_buffer_size_ > 1) {
      resize_buffer(actual_buffer_size_ / 2);
    }

    if (size_ == 0) {
      active_index_.reset();
    } else if (active_index_.has_value() && active_index_.value() >= size_) {
      active_index_ = size_ - 1;
    }
  };

 private:
  // It doesnt copy the data from the old buffer to the new buffer since
  // uniforms are set every frame
  void resize_buffer(uint64_t newSize) {
    auto& context = WgpuContext::get();
    wgpu::Device device = context.get_device();

    wgpu::BufferDescriptor buffer_desc = wgpu::Default;
    buffer_desc.mappedAtCreation = false;
    buffer_desc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform;
    buffer_desc.size = kStride * newSize;
    buffer_desc.label = "DynamicUniformBuffer";

    auto new_buffer = device.createBuffer(buffer_desc);
    storage_buffer_ = new_buffer;
    actual_buffer_size_ = newSize;
  }

  RaiiWrapper<wgpu::Buffer> storage_buffer_;
  std::optional<uint32_t> active_index_;
  uint64_t size_ = 0;
  uint64_t actual_buffer_size_ = 0;
};

}  // namespace renderer