#pragma once
#include <type_traits>

#include "bind_group.hpp"

namespace renderer {
struct PipelineEntries {
  template <typename... T>
    requires(sizeof...(T) <= 4 && (std::is_same_v<T, BindGroup> && ...))
  static auto from_sequential(T&... bindGroup) {
    return std::forward_as_tuple(bindGroup...);
  }
};

}  // namespace renderer