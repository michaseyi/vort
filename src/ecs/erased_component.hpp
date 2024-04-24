#pragma once

#include <cstdint>
#include <functional>
#include <iostream>
#include <vector>

#include "component_storage.hpp"
namespace ecs {

class ErasedComponentStorage {
 public:
  using Deleter = std::function<void(void*)>;
  using Cloner =
      std::function<void(ErasedComponentStorage&, ErasedComponentStorage&)>;
  using Copier = std::function<void(ErasedComponentStorage&, uint32_t, uint32_t,
                                    ErasedComponentStorage&)>;
  using Remover = std::function<void(ErasedComponentStorage&, uint32_t)>;

  ErasedComponentStorage() = default;

  ErasedComponentStorage(void* raw_ptr, Deleter deleter, Cloner cloner,
                         Copier copier, Remover remover)
      : raw_(raw_ptr),
        deleter_(deleter),
        cloner_(cloner),
        copier_(copier),
        remover_(remover) {}

  ErasedComponentStorage(ErasedComponentStorage&& rhs);

  ErasedComponentStorage& operator=(ErasedComponentStorage&& rhs);

  ~ErasedComponentStorage();

  template <typename T>
  ComponentStorage<T>* cast();

  template <typename T>
  static ErasedComponentStorage create();

  void copy_from(uint32_t src_row_index, uint32_t dst_row_index,
                 ErasedComponentStorage& src);

  void remove(uint32_t row_index);

  void clone_to(ErasedComponentStorage& retval);

 private:
  void* raw_ = nullptr;
  Deleter deleter_;
  Cloner cloner_;
  Copier copier_;
  Remover remover_;
};

}  // namespace ecs

#define ERASED_COMPONENT_STORAGE_TEMPLATE_IMPL
#include "erased_component.cpp"
#undef ERASED_COMPONENT_STORAGE_TEMPLATE_IMPL