#pragma once
#include <cassert>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <type_traits>
#include <cstdint>

#include "archtype.hpp"
#include "array_hashmap.hpp"

namespace ecs {

template <typename... T>
class Query;

struct AppState {
  std::string initialization_stage;
  bool initialized;
  bool running;
};

// TODO: Temporary Entity ID solution. Replace with UUIDs;
using EntityId = uint32_t;

enum class SystemSchedule {
  Startup,
  Update,
  Shutdown,
};

template <typename...>
struct UniqueTypes : std::true_type {};

template <typename T, typename... Rest>
struct UniqueTypes<T, Rest...>
    : std::integral_constant<bool, (!std::is_same_v<T, Rest> && ...) &&
                                       UniqueTypes<Rest...>::value> {};

enum class EntityInterface : uint8_t {
  None = 0,
  Mesh,
  Camera,
  Light,
  Scene,
  SkinnedMesh,
  Joint
};

class Entities {
 public:
  template <typename... Args>
  using System = void (*)(Args...);

  using Plugin = std::function<void(Entities&)>;

  using InitComponentHandler = std::function<void(Entities&, EntityId)>;

  using DeinitComponentHandler = InitComponentHandler;

  struct Pointer {
    uint16_t archtype_index;
    uint32_t row_index;
    std::string name;
    EntityInterface interface;
  };

  const static inline uint64_t kVoidArchtypeHash =
      std::numeric_limits<uint64_t>::max();

  const static inline uint32_t kRootEntityId = 0;

  Entities(const Entities&) = delete;

  Entities();

  Entities& operator()(const Entities&) = delete;

  EntityId get_parent(EntityId entityID);

  std::string& get_name(EntityId entityID);

  EntityInterface get_interface(EntityId entityID);

  const std::vector<EntityId>& get_children(
      EntityId parent_id = Entities::kRootEntityId);

  EntityId create_entity(std::string name, EntityInterface interface,
                         EntityId parent_id = Entities::kRootEntityId);

  template <typename... T>
  EntityId create_entity_with(std::string name, EntityInterface interface,
                              EntityId parent_id, T... components);

  void remove_entity(EntityId entityID);

  template <typename... T>
  void set_components(EntityId entityID, T... components);

  template <typename... T>
  std::tuple<T&...> get_components(EntityId entityID);

  template <typename... T>
  void remove_components(EntityId entityID);

  /**
     * @brief The handler is called before the componet is added to the entity
     */
  template <typename T>
  void set_component_init_handler(InitComponentHandler handler);

  /**
     * @brief The handler is called before the componet is removed from the
     * entity
     */
  template <typename T>
  void set_component_deinit_handler(DeinitComponentHandler handler);

  std::vector<ArchTypeStorage>& get_archtypes();

  void run();

  template <typename... T>
  Entities& add_plugins(Plugin plugin, T... rest);

  template <typename... Args, typename... Rest>
  Entities& add_systems(SystemSchedule schedule, System<Args...> system,
                        Rest... rest);

  template <typename... T>
  void set_global(T... args);

  template <typename... T>
  std::tuple<T&...> get_global();

  template <typename... T>
  Query<T...> query();

  void update();

  uint32_t get_entity_count();

  template <typename... T>
  bool has_components(EntityId entityID);

  template <typename T>
  void traverse(std::function<T(Entities&, EntityId, T)>, T,
                EntityId rootEntityID = Entities::kRootEntityId);

  EntityId get_entity_by_name(std::string_view name);

 private:
  void remove_entity_impl(EntityId entityID, bool detach_from_parent);
  EntityId new_entity();
  ArchTypeStorage& arctype_from_entity_id(EntityId entity_id);
  uint32_t get_entity_row_index_from_id(EntityId entityID);

  void run_deinit(EntityId entity_id,
                  std::vector<std::type_index>& component_types);
  void run_init(EntityId entity_id,
                std::vector<std::type_index>& component_types);

  template <typename... T>
  void run_init(EntityId entity_id);

  uint32_t entity_count_ = 0;
  uint32_t next_entity_id_ = 1;

  std::unordered_set<EntityId> reusable_entity_ids_;
  std::unordered_map<EntityId, Pointer> entities_;
  std::map<std::string, EntityId> entity_name_index_;
  ArrayHashMap<uint64_t, ArchTypeStorage> archtypes_;

  std::unordered_map<std::type_index, std::shared_ptr<void>> global_variables_;

  std::vector<std::function<void(void)>> startup_systems_;
  std::vector<std::function<void(void)>> update_systems_;
  std::vector<std::function<void(void)>> shutdown_systems_;

  std::vector<Plugin> plugins_;

  std::map<EntityId, std::vector<EntityId>> entity_children_map_;
  std::map<EntityId, EntityId> entity_parent_map_;

  std::map<std::type_index, InitComponentHandler> component_init_handlers_;
  std::map<std::type_index, DeinitComponentHandler> component_deinit_handlers_;
};

using World = Entities;

}  // namespace ecs

#define ENTITIES_TEMPLATE_IMPL
#include "entities.cpp"
#undef ENTITIES_TEMPLATE_IMPL