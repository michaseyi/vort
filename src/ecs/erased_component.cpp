#include "erased_component.hpp"
namespace ecs {

#ifdef ERASED_COMPONENT_STORAGE_TEMPLATE_IMPL
template <typename T>
ComponentStorage<T>* ErasedComponentStorage::cast() {
  return reinterpret_cast<ComponentStorage<T>*>(raw_);
}

template <typename T>
ErasedComponentStorage ErasedComponentStorage::create() {
  auto component_storage = new ComponentStorage<T>();
  auto deleter = [](void* raw_ptr) {
    delete reinterpret_cast<ComponentStorage<T>*>(raw_ptr);
  };

  auto cloner = [](ErasedComponentStorage& src,
                   ErasedComponentStorage& retval) {
    auto new_component_storage = new ComponentStorage<T>();
    retval = ErasedComponentStorage{
        reinterpret_cast<void*>(new_component_storage), src.deleter_,
        src.cloner_, src.copier_, src.remover_};
  };

  auto copy = [](ErasedComponentStorage& dst, uint32_t src_row_index,
                 uint32_t dst_row_index, ErasedComponentStorage& src) {
    ComponentStorage<T>& destination = *dst.cast<T>();
    ComponentStorage<T>& source = *src.cast<T>();
    destination.copy_from(src_row_index, dst_row_index, source);
  };

  auto remove = [](ErasedComponentStorage& src, uint32_t row) {
    ComponentStorage<T>& source = *src.cast<T>();
    source.remove(row);
  };

  return ErasedComponentStorage{reinterpret_cast<void*>(component_storage),
                                deleter, cloner, copy, remove};
}

#else
ErasedComponentStorage::ErasedComponentStorage(ErasedComponentStorage&& rhs) {
  this->~ErasedComponentStorage();
  deleter_ = rhs.deleter_;
  cloner_ = rhs.cloner_;
  remover_ = rhs.remover_;
  copier_ = rhs.copier_;
  raw_ = rhs.raw_;
  rhs.raw_ = nullptr;
}

ErasedComponentStorage& ErasedComponentStorage::operator=(
    ErasedComponentStorage&& rhs) {
  this->~ErasedComponentStorage();
  deleter_ = rhs.deleter_;
  cloner_ = rhs.cloner_;
  remover_ = rhs.remover_;
  copier_ = rhs.copier_;
  raw_ = rhs.raw_;
  rhs.raw_ = nullptr;
  return *this;
}

ErasedComponentStorage::~ErasedComponentStorage() {
  if (raw_) {
    deleter_(raw_);
  }
}

void ErasedComponentStorage::copy_from(uint32_t src_row_index, uint32_t dst_row_index,
                                       ErasedComponentStorage& src) {
  copier_(*this, src_row_index, dst_row_index, src);
}

void ErasedComponentStorage::remove(uint32_t row_index) {
  remover_(*this, row_index);
}

void ErasedComponentStorage::clone_to(ErasedComponentStorage& retval) {
  cloner_(*this, retval);
}
#endif
}  // namespace ecs