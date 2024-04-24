#pragma once
#include <cassert>
#include <cstdint>
#include <iostream>
#include <typeindex>
#include <typeinfo>
#include <unordered_set>
#include <vector>

#include "array_hashmap.hpp"
#include "erased_component.hpp"

namespace ecs {

using EntityId = uint32_t;
class ArchTypeStorage {
 public:
  uint32_t new_row(EntityId entity);

  template <typename T>
  bool has_component();

  bool has_component(std::type_index type);

  template <typename... T>
  static u_int64_t compute_hash();

  static u_int64_t compute_hash(std::vector<std::type_index>& types);

  template <typename... T>
  static ArchTypeStorage create();

  void compute_hash();

  void set_hash(u_int64_t hash);

  u_int64_t get_hash();

  std::vector<EntityId>& entities();

  void remove_row(uint32_t row_index);

  std::vector<std::type_index>& get_component_types();

  static ArchTypeStorage from_type_index(std::vector<std::type_index>& types);

  ArchTypeStorage clone();

  ArchTypeStorage clone(std::vector<std::type_index>& type);

  template <typename T>
  void set(uint32_t row_index, T component);

  template <typename T>
  void set(T component);

  ArrayHashMap<std::type_index, ErasedComponentStorage>&
  get_component_storages();

  template <typename... T>
  bool has_components();

  void add_component(std::type_index type);

  void add_component(std::type_index type,
                     ErasedComponentStorage erased_component_storage);

  ErasedComponentStorage* get_component(std::type_index& type);

  template <typename T>
  T& get_row(uint32_t row_index);

  EntityId get_entity_id_from_row_index(uint32_t row_index);

 private:
  u_int64_t hash_ = 0;
  
  std::vector<EntityId> entity_ids_;
  std::vector<std::type_index> component_types_;
  ArrayHashMap<std::type_index, ErasedComponentStorage>
      erased_component_storages_;

  // TODO: Use a set for tag components since they require no data. Treating
  // them as regular components would waste 1 byte per entity that is assigned
  // the tag.
  std::unordered_set<std::type_index> tag_components_;
};

}  // namespace ecs

#define ARCHTYPE_TEMPLATE_IMPL
#include "archtype.cpp"
#undef ARCHTYPE_TEMPLATE_IMPL