#pragma once

#include <string_view>

#include "bind_group_entries.hpp"
#include "src/utils/raii.hpp"
#include "wgpu_context.hpp"
namespace renderer {
class Texture {
 public:
  Texture(std::string path);

  Texture() = default;

  operator BindGroupEntries::EntryData();

 private:
  RaiiWrapper<wgpu::Texture> texture_;
};
}  // namespace renderer