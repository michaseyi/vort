
#if defined(EMSCRIPTEN)
#include <emscripten/emscripten.h>

#endif

#include <set>

#include "command.hpp"
#include "entities.hpp"

namespace ecs {

#ifdef ENTITIES_TEMPLATE_IMPL

template <typename T>
void Entities::set_component_init_handler(InitComponentHandler handler) {
  auto type_index = std::type_index(typeid(T));

  component_init_handlers_[type_index] = std::move(handler);
}

template <typename T>
void Entities::set_component_deinit_handler(DeinitComponentHandler handler) {
  auto type_index = std::type_index(typeid(T));

  component_deinit_handlers_[type_index] = std::move(handler);
}

template <typename... Args, typename... Rest>
Entities& Entities::add_systems(SystemSchedule schedule, System<Args...> system,
                                Rest... rest) {
  static_assert(
      (std::is_default_constructible_v<std::remove_reference_t<Args>> && ...));
  auto system_caller = [this, system]() {
    if constexpr (sizeof...(Args) > 0) {
      auto args = std::make_tuple(std::remove_reference_t<Args>()...);
      (std::get<std::remove_reference_t<Args>>(args).fill(this), ...);
      system(std::get<std::remove_reference_t<Args>>(args)...);
    } else {
      (void)this;
      system();
    }
  };

  switch (schedule) {
    case SystemSchedule::Startup:
      startup_systems_.emplace_back(system_caller);
      break;
    case SystemSchedule::Update:
      update_systems_.emplace_back(system_caller);
      break;
    case SystemSchedule::Shutdown:
      shutdown_systems_.emplace_back(system_caller);
      break;
  }

  if constexpr (sizeof...(Rest) > 0) {
    add_systems(schedule, rest...);
  }
  return *this;
}

template <typename... T>
Query<T...> Entities::query() {
  Query<T...> result{};

  result.fill(this);
  return result;
}

template <typename... T>
void Entities::set_global(T... args) {
  (
      [&]() {
        auto index = std::type_index(typeid(T));
        if (auto iter = global_variables_.find(index);
            iter != global_variables_.end()) {
          *(reinterpret_cast<T*>(iter->second.get())) = std::move(args);
        } else {
          global_variables_[std::type_index(typeid(T))] =
              std::make_shared<T>(std::move(args));
        }
      }(),
      ...);
}

template <typename... T>
Entities& Entities::add_plugins(Plugin plugin, T... rest) {
  plugins_.push_back(std::move(plugin));

  if constexpr (sizeof...(T) > 0) {
    add_plugins(rest...);
  }
  return *this;
}

template <typename... T>
std::tuple<T&...> Entities::get_global() {
  auto global_variables_exists =
      ((global_variables_.find(std::type_index(typeid(T))) !=
        global_variables_.end()) &&
       ...);

  assert(global_variables_exists);

  return std::make_tuple<std::reference_wrapper<T>...>(*reinterpret_cast<T*>(
      global_variables_[std::type_index(typeid(T))].get())...);
}

template <typename... T>
void Entities::set_components(EntityId entity_id, T... components) {
  static_assert(sizeof...(T) > 0,
                "There should be at least one component to be added");

  static_assert(UniqueTypes<T...>::value, "Component Types must be unique");

  auto& entity_current_archtype = arctype_from_entity_id(entity_id);

  // To be used to access entityArchType once mArchtypes has been mutated
  auto entity_current_archtype_index =
      archtypes_.get_dense_storage_index(entity_current_archtype.get_hash());

  auto entity_row_index_in_current_archtype =
      get_entity_row_index_from_id(entity_id);

  if (entity_current_archtype.has_components<T...>()) {
    (entity_current_archtype.set<T>(entity_row_index_in_current_archtype,
                                    std::move(components)),
     ...);
  } else {
    // This has to be done incase some of the component to be added already
    // exists in the current archtype of the entity
    auto unique_component_types =
        std::set<std::type_index>{std::type_index(typeid(T))...};

    // call component init handler on new components to be added
    for (auto& type : unique_component_types) {
      if (!entity_current_archtype.has_component(type)) {
        auto component_init_handler = component_init_handlers_.find(type);
        if (component_init_handler == component_init_handlers_.end()) {
          continue;
        }
        component_init_handler->second(*this, entity_id);
      }
    }

    for (auto& type : entity_current_archtype.get_component_types()) {
      unique_component_types.emplace(type);
    }

    std::vector<std::type_index> entity_new_archtype_components(
        unique_component_types.begin(), unique_component_types.end());

    auto entity_new_archtype_hash =
        ArchTypeStorage::compute_hash(entity_new_archtype_components);

    ArchTypeStorage* entity_new_archtype =
        archtypes_.get(entity_new_archtype_hash);

    if (entity_new_archtype == nullptr) {
      auto archtype = entity_current_archtype.clone();

      // add component storage from the template
      (archtype.add_component(std::type_index(typeid(T)),
                              ErasedComponentStorage::create<T>()),
       ...);

      archtype.set_hash(entity_new_archtype_hash);
      archtypes_.put(entity_new_archtype_hash, std::move(archtype));
      entity_new_archtype = archtypes_.get(entity_new_archtype_hash);
    }

    auto entity_row_index_in_new_archtype =
        entity_new_archtype->new_row(entity_id);

    // The previous entity archtype has to be accessed this way because the
    // reference we had earlier would already be invalidated by the
    // archtypes_.put call.
    auto& entity_prev_archtype =
        archtypes_.values()[entity_current_archtype_index];

    auto entity_row_index_in_prev_archtype =
        get_entity_row_index_from_id(entity_id);

    //  copy the entity row from the entities previouse archtype
    for (auto type : entity_prev_archtype.get_component_types()) {
      entity_new_archtype->get_component(type)->copy_from(
          entity_row_index_in_prev_archtype, entity_row_index_in_new_archtype,
          *entity_prev_archtype.get_component(type));
    }
    // copy component values from the input to this method
    (entity_new_archtype->template set<T>(std::move(components)), ...);

    auto entity_new_archtype_index =
        archtypes_.get_dense_storage_index(entity_new_archtype_hash);
    auto& entity_ptr = entities_[entity_id];
    entity_ptr.archtype_index = entity_new_archtype_index;
    entity_ptr.row_index = entity_row_index_in_new_archtype;

    // update the row index of the last entity in the previous archtypestorage
    // if it was not the one that was removed.
    if (auto last_entity_id = entity_prev_archtype.entities().back();
        last_entity_id != entity_id) {
      entities_[last_entity_id].row_index = entity_row_index_in_prev_archtype;
    }
    entity_prev_archtype.remove_row(entity_row_index_in_prev_archtype);
  }
}

template <typename... T>
std::tuple<T&...> Entities::get_components(EntityId entity_id) {
  static_assert(sizeof...(T) > 0,
                "There should be at least one component to be retrieved");

  static_assert(UniqueTypes<T...>::value, "Component Types must be unique");

  auto& entity_archtype = arctype_from_entity_id(entity_id);
  auto entity_row_index_in_archtype = get_entity_row_index_from_id(entity_id);

  // TODO: Consider cases when you are sure an entity has the component, maybe due to a previous check. Performing
  // the check a second time would be inefficient
  auto components_exist = entity_archtype.has_components<T...>();
  assert(components_exist);

  return std::make_tuple(std::reference_wrapper<T>(
      entity_archtype.get_row<T>(entity_row_index_in_archtype))...);
}

template <typename... T>
void Entities::remove_components(EntityId entity_id) {
  static_assert(sizeof...(T) > 0,
                "There should be at least one component to be removed");

  static_assert(UniqueTypes<T...>::value, "Component Types must be unique");

  auto& entity_current_archtype = arctype_from_entity_id(entity_id);

  auto entity_current_archtype_index =
      archtypes_.get_dense_storage_index(entity_current_archtype.get_hash());

  bool components_exist = entity_current_archtype.has_components<T...>();

  assert(components_exist);

  std::set<std::type_index> components_to_remove = {
      std::type_index(typeid(T))...};

  // call component deinit handler for components to be removed
  for (auto& type : components_to_remove) {
    auto component_deinit_handler = component_deinit_handlers_.find(type);
    if (component_deinit_handler == component_deinit_handlers_.end()) {
      continue;
    }
    component_deinit_handler->second(*this, entity_id);
  }

  std::vector<std::type_index> component_to_remain_with_entity;

  for (auto& type : entity_current_archtype.get_component_types()) {
    if (components_to_remove.find(type) == components_to_remove.end()) {
      component_to_remain_with_entity.emplace_back(type);
    }
  }

  auto new_archtype_hash =
      ArchTypeStorage::compute_hash(component_to_remain_with_entity);

  ArchTypeStorage* entity_new_archtype = archtypes_.get(new_archtype_hash);

  if (entity_new_archtype == nullptr) {
    auto archtype =
        entity_current_archtype.clone(component_to_remain_with_entity);

    archtype.set_hash(new_archtype_hash);
    archtypes_.put(new_archtype_hash, std::move(archtype));
    entity_new_archtype = archtypes_.get(new_archtype_hash);
  }

  auto& entity_prev_archtype =
      archtypes_.values()[entity_current_archtype_index];

  auto entity_row_index_in_prev_archtype =
      get_entity_row_index_from_id(entity_id);

  auto entity_row_index_in_new_archtype =
      entity_new_archtype->new_row(entity_id);

  for (auto type : component_to_remain_with_entity) {
    entity_new_archtype->get_component(type)->copy_from(
        entity_row_index_in_prev_archtype, entity_row_index_in_new_archtype,
        *entity_prev_archtype.get_component(type));
  }

  auto entity_new_archtype_index =
      archtypes_.get_dense_storage_index(new_archtype_hash);
  auto& entity_ptr = entities_[entity_id];
  entity_ptr.archtype_index = entity_new_archtype_index;
  entity_ptr.row_index = entity_row_index_in_new_archtype;

  // update the row index of the last entity in the previous archtypestorage if
  // it was not the one that was removed.
  if (auto last_entity_id = entity_prev_archtype.entities().back();
      last_entity_id != entity_id) {
    entities_[last_entity_id].row_index = entity_row_index_in_prev_archtype;
  }
  entity_prev_archtype.remove_row(entity_row_index_in_prev_archtype);
}

template <typename T>
void Entities::traverse(std::function<T(Entities&, EntityId, T)> callback,
                        T start_value, EntityId root_entity_id) {
  auto& children = get_children(root_entity_id);

  auto updated_value = root_entity_id == Entities::kRootEntityId
                           ? start_value
                           : callback(*this, root_entity_id, start_value);

  for (auto child : children) {
    traverse(callback, updated_value, child);
  }
}

template <typename... T>
bool Entities::has_components(EntityId entity_id) {
  auto& archtype = arctype_from_entity_id(entity_id);
  return archtype.has_components<T...>();
}

#else
std::string& Entities::get_name(EntityId entity_id) {
  assert(entities_.find(entity_id) != entities_.end() &&
         "Entity does not exist in this world.");
  return entities_[entity_id].name;
}

EntityInterface Entities::get_interface(EntityId entity_id) {
  assert(entities_.find(entity_id) != entities_.end() &&
         "Entity does not exist in this world.");
  return entities_[entity_id].interface;
}

Entities::Entities() {
  archtypes_.put(Entities::kVoidArchtypeHash, ArchTypeStorage{});

  set_global(AppState{.initialization_stage = "Loading Scene",
                      .initialized = false,
                      .running = true});
};

EntityId Entities::new_entity(std::string name, EntityInterface interface,
                              EntityId parent_id) {
  assert(entity_name_index_.find(name) == entity_name_index_.end());

  entity_count_++;

  EntityId new_entity_id;

  if (reusable_entity_ids_.size() > 0) {
    new_entity_id = *reusable_entity_ids_.begin();

    reusable_entity_ids_.erase(new_entity_id);
  } else {
    new_entity_id = next_entity_id_++;
  }

  auto void_archtype = archtypes_.get(Entities::kVoidArchtypeHash);
  auto new_entity_row_index = void_archtype->new_row(new_entity_id);
  entities_[new_entity_id] = {.archtype_index = 0,
                              .row_index = new_entity_row_index,
                              .name = name,
                              .interface = interface};

  entity_name_index_[name] = new_entity_id;

  entity_children_map_[parent_id].push_back(new_entity_id);

  entity_parent_map_[new_entity_id] = parent_id;
  return new_entity_id;
}

void Entities::update() {
  for (auto& system : update_systems_) {
    system();
  }
}

void Entities::run() {
  auto [app_state] = get_global<AppState>();
  app_state.initialization_stage = "Building plugins";

  // build plugins
  for (auto& plugin : plugins_) {
    plugin(*this);
  }

  app_state.initialization_stage = "Running startup systems";

  for (auto& system : startup_systems_) {
    system();
  }

  app_state.initialization_stage = "Engine initialized";
  app_state.initialized = true;
#if defined(EMSCRIPTEN)
  emscripten_set_main_loop_arg(
      [](void* user_data) {
        Entities& world = *reinterpret_cast<Entities*>(user_data);
        world.update();
      },
      (void*)this, 0, true);
#else

  while (appState.running) {
    update();
  }

  for (auto& system : shutdown_systems_) {
    system();
  }

#endif
}

void Entities::remove_entity(EntityId entity_id) {
  assert(entity_id != Entities::kRootEntityId);

  remove_entity_impl(entity_id, true);
}

void Entities::remove_entity_impl(EntityId entity_id, bool detach_from_parent) {
  auto entity_children_iter = entity_children_map_.find(entity_id);

  if (entity_children_iter != entity_children_map_.end()) {
    for (auto child : entity_children_iter->second) {
      remove_entity_impl(child, false);
    }
  }

  auto& entity_archtype = arctype_from_entity_id(entity_id);

  // call component deinit handler for all componets of entity to be removed
  for (auto& type : entity_archtype.get_component_types()) {
    auto component_deinit_handler = component_deinit_handlers_.find(type);
    if (component_deinit_handler == component_deinit_handlers_.end()) {
      continue;
    }
    component_deinit_handler->second(*this, entity_id);
  }

  auto& entity_ptr = entities_[entity_id];

  if (auto last_entity_id = entity_archtype.entities().back();
      last_entity_id != entity_id) {
    entities_[last_entity_id].row_index = entity_ptr.row_index;
  }

  entity_archtype.remove_row(entity_ptr.row_index);

  entity_count_--;

  if (detach_from_parent) {
    auto& entity_parent_children =
        entity_children_map_[entity_parent_map_[entity_id]];

    std::array<EntityId, 1> entity_to_search = {entity_id};

    auto entity_iter_in_parent = std::search(
        entity_parent_children.begin(), entity_parent_children.end(),
        entity_to_search.begin(), entity_to_search.end());

    entity_parent_children.erase(entity_iter_in_parent);
  }
  entity_name_index_.erase(entity_ptr.name);
  entities_.erase(entity_id);
  entity_children_map_.erase(entity_id);
  entity_parent_map_.erase(entity_id);

  reusable_entity_ids_.emplace(entity_id);
}

EntityId Entities::get_parent(EntityId entity_id) {
  return entity_parent_map_[entity_id];
}

uint32_t Entities::get_entity_count() {
  return entity_count_;
}

const std::vector<EntityId>& Entities::get_children(EntityId entity_id) {
  return entity_children_map_[entity_id];
}

std::vector<ArchTypeStorage>& Entities::get_archtypes() {
  return archtypes_.values();
}

ArchTypeStorage& Entities::arctype_from_entity_id(EntityId entity_id) {
  auto entity_iter = entities_.find(entity_id);

  assert(entity_iter != entities_.end());

  return archtypes_.values()[entity_iter->second.archtype_index];
}

uint32_t Entities::get_entity_row_index_from_id(EntityId entity_id) {
  auto entity_iter = entities_.find(entity_id);

  assert(entity_iter != entities_.end());

  return entity_iter->second.row_index;
}
#endif
}  // namespace ecs