#pragma once
#include <cstdint>
#include <iostream>

namespace ecs {

template <typename... T>
class Global {
 public:
  Global(){};
  void fill(Entities* world) {
    world_ = world;
    auto globals = world_->get_global<T...>();

    (
        [&]() {
          std::get<T*>(values_) = &std::get<T&>(globals);
        }(),
        ...);
  }

  const std::tuple<T*...>& values() const { return values_; }

 private:
  std::tuple<T*...> values_;
  Entities* world_;
};

template <size_t I, typename... T>
auto& get(const ecs::Global<T...>& obj) {
  return *std::get<I>(obj.values());
}

}  // namespace ecs

template <typename... T>
struct std::tuple_size<ecs::Global<T...>>
    : std::integral_constant<size_t, sizeof...(T)> {};

template <size_t I, typename... T>
struct std::tuple_element<I, ecs::Global<T...>> {
  using type = decltype(std::get<I>(std::tuple<T...>()))&;
};