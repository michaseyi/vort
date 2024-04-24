#include "uniforms.hpp"

#include <cassert>
#include <iostream>

#include "resource.hpp"
#include "src/utils/raii.hpp"
#include "wgpu_context.hpp"
namespace renderer {

uint32_t Uniform::ceil_to_next_multiple(uint32_t value, uint32_t step) {
  uint32_t divice_and_ceil = value / step + (value % step == 0 ? 0 : 1);

  return step * divice_and_ceil;
}

uint32_t Uniform::compute_stride(uint32_t original_size) {
  auto& context = WgpuContext::get();
  auto device = context.get_device();

  wgpu::SupportedLimits limits;
  device.getLimits(&limits);

  uint32_t min_uniform_buffer_offset_alignment =
      limits.limits.minUniformBufferOffsetAlignment;

  return ceil_to_next_multiple(original_size,
                               min_uniform_buffer_offset_alignment);
}

Uniform::Uniform(size_t size) : Uniform(size, false){};

Uniform::Uniform(size_t size, bool has_dynamic_offset)
    : size_(size), has_dynamic_offset_(has_dynamic_offset) {
  assert(size_ % 16 == 0 && "Uniform type must be a multiple of 16");
  if (!has_dynamic_offset_) {
    buffer_length_used_ = 1;
    stride_ = size_;
  } else {
    stride_ = compute_stride(size_);
  }
  resize(1);
}

void Uniform::set(size_t size, void* data) {
  set(0, size_, data);
}

void Uniform::set(size_t offset, size_t size, void* data) {
  set(0, offset, size, data);
}

void Uniform::set(size_t index, size_t offset, size_t size, void* data) {
  assert(buffer_length_used_ > index);

  assert(*storage_buffer_);

  auto& context = WgpuContext::get();
  auto queue = context.get_queue();
  queue.writeBuffer(*storage_buffer_, (stride_ * index) + offset, data, size);
}

void Uniform::increment() {
  assert(has_dynamic_offset_);
  buffer_length_used_++;

  if (buffer_length_used_ > buffer_length_) {
    size_t newLength = buffer_length_ * 2;
    resize(newLength);
  }

  assert(!(buffer_length_used_ > buffer_length_));
}

void Uniform::decrement() {
  assert(has_dynamic_offset_);

  buffer_length_used_--;

  if (buffer_length_used_ < buffer_length_ / 2 &&
      (buffer_length_ - buffer_length_used_) > 8) {
    size_t newLength = buffer_length_ / 2;
    resize(newLength);
  }
}

ResourceEntry Uniform::resource_entry() const {
  ResourceEntry entry{};

  entry.bind_group_entry.size = size_;
  entry.bind_group_entry.buffer = *storage_buffer_;
  entry.bind_group_entry.offset = 0;

  entry.bind_group_layout_entry.buffer.hasDynamicOffset = has_dynamic_offset_;
  entry.bind_group_layout_entry.buffer.type = wgpu::BufferBindingType::Uniform;

  entry.bind_group_layout_entry.visibility = wgpu::ShaderStage::Compute |
                                             wgpu::ShaderStage::Fragment |
                                             wgpu::ShaderStage::Vertex;
  return entry;
}

size_t Uniform::size() const {
  return buffer_length_;
}

uint32_t Uniform::stride() const {
  return stride_;
}

void Uniform::resize(size_t size) {
  wgpu::BufferDescriptor buffer_desc = wgpu::Default;
  buffer_desc.mappedAtCreation = false;
  buffer_desc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform;
  buffer_desc.size = stride_ * size;

  auto& context = WgpuContext::get();
  auto device = context.get_device();

  auto new_buffer = device.createBuffer(buffer_desc);

  storage_buffer_ = new_buffer;
  buffer_length_ = size;
}

}  // namespace renderer