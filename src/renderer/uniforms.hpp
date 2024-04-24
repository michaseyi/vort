
#pragma once

#include <cassert>
#include <iostream>

#include "resource.hpp"
#include "src/utils/raii.hpp"
#include "wgpu_context.hpp"

namespace renderer {

class Uniform : public Resource {
 public:
  uint32_t ceil_to_next_multiple(uint32_t value, uint32_t step);

  uint32_t compute_stride(uint32_t original_size);

  Uniform(size_t size);

  Uniform(size_t size, bool has_dynamic_offset);

  void set(size_t size, void* data);

  void set(size_t offset, size_t size, void* data);

  void set(size_t index, size_t offset, size_t size, void* data);

  void increment();

  void decrement();

  ResourceEntry resource_entry() const override;

  size_t size() const;

  uint32_t stride() const;

 private:
  void resize(size_t size);

  size_t buffer_length_used_ = 0;
  size_t buffer_length_ = 0;
  RaiiWrapper<wgpu::Buffer> storage_buffer_;
  bool has_dynamic_offset_;
  size_t stride_ = 0;

  size_t size_ = 0;
};
}  // namespace renderer