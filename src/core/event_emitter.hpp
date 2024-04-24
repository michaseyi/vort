#pragma once

#include <functional>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>

namespace core {

using EventListener = std::function<void(const void*)>;

class EventEmitter {
 public:
  template <typename T>
  void register_listener(std::function<void(const T*)> listener) {
    auto type_index = std::type_index(typeid(T));

    listeners_[type_index].emplace_back([listener](const void* raw) {
      listener(reinterpret_cast<const T*>(raw));
    });
  }

  template <typename T>
  void emit_event(T event_data) {
    auto type_index = std::type_index(typeid(T));

    auto listeners_iter = listeners_.find(type_index);

    if (listeners_iter == listeners_.end()) {
      return;
    }

    for (auto& listener : listeners_iter->second) {
      listener(&event_data);
    }
  }
  // void unregisterListener();

 private:
  std::unordered_map<std::type_index, std::vector<EventListener>> listeners_;
};

}  // namespace core