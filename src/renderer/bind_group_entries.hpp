#pragma once
#include "wgpu_context.hpp"

namespace renderer {
struct BindGroupEntries {
  std::vector<wgpu::BindGroupEntry> entries;
  std::vector<wgpu::BindGroupLayoutEntry> layout_entries;

  struct EntryData {
    wgpu::BindGroupEntry entry = wgpu::Default;
    wgpu::BindGroupLayoutEntry layout_entry = wgpu::Default;
  };

  struct IndexedEntry {
    uint32_t binding;
    EntryData entry;
  };

  static BindGroupEntries from_sequential(std::vector<EntryData> resources) {
    BindGroupEntries entries;
    entries.entries.reserve(resources.size());
    entries.layout_entries.reserve(resources.size());

    for (uint32_t i = 0; i < resources.size(); i++) {
      entries.entries.emplace_back(resources[i].entry);
      entries.layout_entries.emplace_back(resources[i].layout_entry);

      entries.entries[i].binding = i;
      entries.layout_entries[i].binding = i;
    }
    return entries;
  }

  static BindGroupEntries from_indexed(std::vector<IndexedEntry> resources) {
    BindGroupEntries entries;
    entries.entries.reserve(resources.size());
    entries.layout_entries.reserve(resources.size());

    for (uint32_t i = 0; i < resources.size(); i++) {
      entries.entries.emplace_back(resources[i].entry.entry);
      entries.layout_entries.emplace_back(resources[i].entry.layout_entry);

      entries.entries[i].binding = resources[i].binding;
      entries.layout_entries[i].binding = resources[i].binding;
    }
    return entries;
  };
};

}  // namespace renderer
