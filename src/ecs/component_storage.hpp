#pragma once
#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>

namespace ecs {
template <typename T>

class ComponentStorage {
  static_assert(std::is_default_constructible_v<T>);

 public:
  uint32_t put(T component);

  void remove(uint32_t row_index);

  void copy_from(uint32_t src_row_index, uint32_t dst_row_index,
                 ComponentStorage<T>& src);

  T& operator[](uint32_t row_index);

 private:
  std::vector<T> entries_;
};

}  // namespace ecs

#define COMPONENT_STORAGE_TEMPLATE_IMPL
#include "component_storage.cpp"
#undef COMPONENT_STORAGE_TEMPLATE_IMPL