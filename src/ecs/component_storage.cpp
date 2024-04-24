#include "component_storage.hpp"
namespace ecs {

#ifdef COMPONENT_STORAGE_TEMPLATE_IMPL
template <typename T>
uint32_t ComponentStorage<T>::put(T component) {
  auto index = entries_.size();
  entries_.emplace_back(std::move(component));
  return index;
}

template <typename T>
void ComponentStorage<T>::remove(uint32_t row_index) {
  if (!(entries_.size() > row_index)) {
    return;
  }

  if (row_index < entries_.size() - 1) {
    std::swap(entries_[row_index], entries_.back());
  }

  entries_.pop_back();
}

template <typename T>
void ComponentStorage<T>::copy_from(uint32_t src_row_index, uint32_t dst_row_index,
                                    ComponentStorage<T>& src) {
  while (entries_.size() <= dst_row_index) {
    entries_.emplace_back();
  }
  entries_[dst_row_index] = std::move(src[src_row_index]);
}

template <typename T>
T& ComponentStorage<T>::operator[](uint32_t row_index) {
  assert(entries_.size() > row_index);
  return entries_[row_index];
}

#else
#endif
}  // namespace ecs