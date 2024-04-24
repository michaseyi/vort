
#include "resource.hpp"

#include <cstdint>
#include <map>
#include <memory>
#include <vector>

#include "src/utils/raii.hpp"
#include "wgpu_context.hpp"

namespace renderer {

ResourceGroupEntry ResourceGroup::resource_group_entry() {
  if (!dirty_) {
    return ResourceGroupEntry{.wgpu_bind_group = bind_group_,
                              .wgpu_bind_group_layout = bind_group_layout};
  }

  auto& context = WgpuContext::get();

  auto device = context.get_device();

  std::vector<wgpu::BindGroupLayoutEntry> bind_group_layout_entries;
  bind_group_layout_entries.reserve(resources_.size());

  std::vector<wgpu::BindGroupEntry> bind_group_entries;
  bind_group_entries.reserve(resources_.size());

  for (std::shared_ptr<Resource>& resource : resources_) {
    ResourceEntry resource_entry = resource->resource_entry();

    uint32_t binding =
        resource_bindings_[reinterpret_cast<uintptr_t>(resource.get())];

    resource_entry.bind_group_entry.binding = binding;
    resource_entry.bind_group_layout_entry.binding = binding;

    bind_group_layout_entries.push_back(resource_entry.bind_group_layout_entry);
    bind_group_entries.push_back(resource_entry.bind_group_entry);
  }

  wgpu::BindGroupLayoutDescriptor bind_group_layout_desc = wgpu::Default;
  bind_group_layout_desc.entryCount = bind_group_layout_entries.size();
  bind_group_layout_desc.entries = bind_group_layout_entries.data();

  bind_group_layout = device.createBindGroupLayout(bind_group_layout_desc);

  wgpu::BindGroupDescriptor bind_group_desc = wgpu::Default;
  bind_group_desc.layout = *bind_group_layout;
  bind_group_desc.entries = bind_group_entries.data();
  bind_group_desc.entryCount = bind_group_entries.size();

  bind_group_ = device.createBindGroup(bind_group_desc);

  dirty_ = false;
  return ResourceGroupEntry{.wgpu_bind_group = bind_group_,
                            .wgpu_bind_group_layout = bind_group_layout};
}

void ResourceGroup::set_binding_count(uint32_t binding_count) {
  resources_.resize(binding_count);
}
void ResourceGroup::bind_resource(uint32_t binding_position,
                                  std::shared_ptr<Resource> resource) {
  assert(resources_.size() > binding_position);

  resource_bindings_[reinterpret_cast<uintptr_t>(resource.get())] =
      binding_position;

  resources_[binding_position] = std::move(resource);

  dirty_ = true;
}

}  // namespace renderer