#pragma once
#include <cstdint>

#include "entities.hpp"
namespace ecs {

class Command {
 public:
  void fill(Entities* world) { world_ = world; }

  template <typename... T>
  Query<T...> query() {
    Query<T...> result{};

    result.fill(world_);
    return result;
  }

  template <typename... T>
  void remove_components(EntityId entity) {
    assert(world_);

    world_->remove_components<T...>(entity);
  }

  template <typename... T>
  void set_global(T... values) {
    assert(world_);
    world_->set_global<T...>(std::move(values)...);
  }

  template <typename... T>
  auto get_global() {
    return world_->get_global<T...>();
  }

  Entities& get_world() { return *world_; }

 private:
  Entities* world_;
};
}  // namespace ecs