#include "archtype.hpp"

namespace ecs {

#ifdef ARCHTYPE_TEMPLATE_IMPL
template <typename T>
void ArchTypeStorage::set(uint32_t row_index, T component) {
  assert(entity_ids_.size() > row_index);

  auto index = std::type_index(typeid(T));
  auto& storage = *erased_component_storages_.get(index);
  ComponentStorage<T>& typed_storage = *storage.cast<T>();
  typed_storage[row_index] = std::move(component);
}

template <typename T>
void ArchTypeStorage::set(T component) {
  auto index = std::type_index(typeid(T));
  auto& storage = *erased_component_storages_.get(index);
  ComponentStorage<T>& typed_storage = *storage.cast<T>();
  auto row = typed_storage.put(std::move(component));
  assert(row == entity_ids_.size() - 1);
}

template <typename... T>
bool ArchTypeStorage::has_components() {
  return (has_component<T>() && ...);
}

template <typename... T>
ArchTypeStorage ArchTypeStorage::create() {
  auto archtype = ArchTypeStorage();

  archtype.hash_ = compute_hash<T...>();

  size_t size = sizeof...(T);

  archtype.erased_component_storages_.reserve(size);
  archtype.component_types_.reserve(size);

  (
      [&]() {
        auto index = std::type_index(typeid(T));
        archtype.component_types_.emplace_back(index);
        archtype.erased_component_storages_.put(
            index, ErasedComponentStorage::create<T>());
      }(),
      ...);
  return archtype;
}

template <typename T>
bool ArchTypeStorage::has_component() {
  auto key = std::type_index(typeid(T));
  return has_component(key);
}

template <typename... T>
u_int64_t ArchTypeStorage::compute_hash() {
  std::vector<std::type_index> types{std::type_index(typeid(T))...};
  return compute_hash(types);
}

template <typename T>
T& ArchTypeStorage::get_row(uint32_t rowIndex) {
  auto index = std::type_index(typeid(T));
  return (*get_component(index)->cast<T>())[rowIndex];
}

#else

uint32_t ArchTypeStorage::new_row(EntityId entity_id) {
  auto new_row_index = entity_ids_.size();
  entity_ids_.emplace_back(entity_id);
  return new_row_index;
}

bool ArchTypeStorage::has_component(std::type_index type) {
  if (erased_component_storages_.get(type)) {
    return true;
  }
  return false;
}

u_int64_t ArchTypeStorage::compute_hash(std::vector<std::type_index>& types) {
  std::sort(types.begin(), types.end(), std::greater<std::type_index>());

  u_int64_t hash = 0;

  for (auto& type : types) {
    hash ^= type.hash_code();
  }
  return hash;
}

void ArchTypeStorage::compute_hash() {
  hash_ = compute_hash(component_types_);
}

void ArchTypeStorage::set_hash(u_int64_t hash) {
  hash_ = hash;
}

u_int64_t ArchTypeStorage::get_hash() {
  return hash_;
}

std::vector<EntityId>& ArchTypeStorage::entities() {
  return entity_ids_;
}

EntityId ArchTypeStorage::get_entity_id_from_row_index(uint32_t row_index) {
  return entity_ids_[row_index];
}

void ArchTypeStorage::remove_row(uint32_t row_index) {
  assert(entity_ids_.size() > row_index);

  std::swap(entity_ids_[row_index], entity_ids_.back());
  entity_ids_.pop_back();
  for (auto& component : erased_component_storages_.values()) {
    component.remove(row_index);
  }
}

std::vector<std::type_index>& ArchTypeStorage::get_component_types() {
  return component_types_;
}

ArchTypeStorage ArchTypeStorage::from_type_index(
    std::vector<std::type_index>& types) {
  ArchTypeStorage archtype;

  for (auto& type : types) {
    archtype.add_component(type);
  }

  return archtype;
}

ArchTypeStorage ArchTypeStorage::clone() {
  auto archtype_clone = from_type_index(component_types_);

  for (auto& type : component_types_) {
    get_component(type)->clone_to(*archtype_clone.get_component(type));
  }

  return archtype_clone;
}

ArchTypeStorage ArchTypeStorage::clone(std::vector<std::type_index>& types) {
  for (auto& type : types) {
    assert(has_component(type));
  }

  auto archtype_clone = from_type_index(types);

  for (auto& type : types) {
    get_component(type)->clone_to(*archtype_clone.get_component(type));
  }

  return archtype_clone;
}

ArrayHashMap<std::type_index, ErasedComponentStorage>&
ArchTypeStorage::get_component_storages() {
  return erased_component_storages_;
}

void ArchTypeStorage::add_component(std::type_index type) {
  if (has_component(type)) {
    return;
  }
  component_types_.emplace_back(type);
  erased_component_storages_.put(type, ErasedComponentStorage());
}

void ArchTypeStorage::add_component(
    std::type_index type, ErasedComponentStorage erased_component_storage) {
  if (has_component(type)) {
    return;
  }
  component_types_.emplace_back(type);
  erased_component_storages_.put(type, std::move(erased_component_storage));
}

ErasedComponentStorage* ArchTypeStorage::get_component(std::type_index& type) {
  return erased_component_storages_.get(type);
}
#endif

}  // namespace ecs