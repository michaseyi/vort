#pragma once
#include <cstdint>
#include <iostream>
#include <type_traits>
#include <typeindex>
#include <typeinfo>

namespace ecs {

using EntityId = uint32_t;

template <typename T>
void dispatch_component_update(EntityId, T&) {
  std::type_index index(typeid(T));
  std::cout << index.name()
            << "Component does not have a dispatch_component_update partial "
               "template"
            << std::endl;
}
}  // namespace ecs
