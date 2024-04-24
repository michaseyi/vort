#pragma once
#include <coroutine>
#include <cstdint>

#include "entities.hpp"

namespace ecs {
template <typename... T>
class Tags {};

template <typename... T>
class Columns {};

template <typename... T>
class Query {
  static_assert(false && "Requires Tag names");
};

template <typename... T, typename... U>
class Query<Columns<T...>, Tags<U...>> {
  static_assert(UniqueTypes<T...>::value);
  static_assert(UniqueTypes<U...>::value);

 public:
  void fill(Entities* world) {
    world_ = world;
    // auto &archtypes = world->archtypes();
  };

  // It is important to note that operations that could create or remove
  // archtypes, create or remove component storages, affect the row index of
  // an entity should not be performed while a query object iterator is in use
  // as it can lead to undefined behaviour. Only after you are done using the
  // iterator is it safe to perform these actions.

  // TODO: Invalidate any iterator when any of the cases above happens.
  // Store a pointer to each iterator, and when any mutation occurs to the
  // Entities, invalidate all stored iterators.
  struct ResultEntry {
    EntityId entityId;
    std::tuple<T*...> components;
  };

  class Iterator {
   public:
    friend class Query<Columns<T...>, Tags<U...>>;
    struct promise_type {
      ResultEntry current_value;
      auto get_return_object() {
        return std::coroutine_handle<promise_type>::from_promise(*this);
      }

      auto initial_suspend() { return std::suspend_always{}; }
      auto final_suspend() noexcept { return std::suspend_always{}; }
      void return_void() {}
      void unhandled_exception() { std::terminate(); }
      auto yield_value(ResultEntry value) {
        current_value = value;
        return std::suspend_always{};
      }
    };

    Iterator(std::coroutine_handle<promise_type> coroutine_handle)
        : coroutine_handle_(coroutine_handle) {}

    auto operator*() {
      return std::tie(*std::get<T*>(
          coroutine_handle_.promise().current_value.components)...);
    }

    auto currentEntityID() {
      return coroutine_handle_.promise().current_value.entityId;
    }

    bool operator==(std::default_sentinel_t) const {
      return coroutine_handle_.done();
    }

    Iterator& operator++() {
      if (!coroutine_handle_.done()) {
        coroutine_handle_.resume();
      }
      return *this;
    }

   private:
    Iterator& begin() {
      if (!coroutine_handle_.done()) {
        coroutine_handle_.resume();
      }
      return *this;
    }

    std::coroutine_handle<promise_type> coroutine_handle_;
  };

  Iterator begin() {
    auto range_generator = [&]() -> Iterator {
      auto& archtypes = world_->get_archtypes();

      for (uint32_t i = 1; i < archtypes.size(); i++) {
        if (archtypes[i].has_components<T..., U...>()) {
          for (auto& entity : archtypes[i].entities()) {
            auto components = world_->get_components<T...>(entity);
            co_yield ResultEntry{entity,
                                 std::make_tuple(&std::get<T&>(components)...)};
          }
        }
      }
    };

    auto range = range_generator();
    range.begin();
    return range;
  }

  std::default_sentinel_t end() { return {}; }

  auto single() { return *begin(); }

 private:
  Entities* world_;
};

}  // namespace ecs
