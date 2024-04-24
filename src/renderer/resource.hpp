#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <vector>

#include "src/utils/raii.hpp"
#include "wgpu_context.hpp"

namespace renderer {

struct ResourceEntry {
  wgpu::BindGroupEntry bind_group_entry;
  wgpu::BindGroupLayoutEntry bind_group_layout_entry;
};

class Resource {
 public:
  virtual ResourceEntry resource_entry() const = 0;

  virtual ~Resource() = default;
};

struct ResourceGroupEntry {
  wgpu::BindGroup wgpu_bind_group;
  wgpu::BindGroupLayout wgpu_bind_group_layout;
};

class ResourceGroup {
 public:
  ResourceGroupEntry resource_group_entry();

  void set_binding_count(uint32_t binding_count);

  void bind_resource(uint32_t binding_position,
                     std::shared_ptr<Resource> resource);

  ResourceGroup() = default;

 private:
  std::vector<std::shared_ptr<Resource>> resources_;
  std::map<uintptr_t, uint32_t> resource_bindings_;

  RaiiWrapper<wgpu::BindGroup> bind_group_;
  RaiiWrapper<wgpu::BindGroupLayout> bind_group_layout;

  bool dirty_ = true;
};
}  // namespace renderer