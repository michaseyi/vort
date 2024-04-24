#pragma once
#include <memory>
#include <string>
#include <string_view>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace core {

template <typename T>
class AssetHandle {
 public:
  using AssetType = T;

 private:
  int32_t mKey = -1;
};

template <typename T>
class AssetLoader {};
class Texture {};

template <>
class AssetLoader<Texture> {
 public:
  static Texture load(const std::string_view path) { return Texture(); }

  static Texture load(Texture&& texture) { return texture; }
};

class AssetManager {
 public:
  template <typename T, typename... Args>
    requires(AssetLoader<T>::load)
  T load(Args... args) {
    return AssetLoader<T>::load(args...);
  }

  template <typename T>
  T get(AssetHandle<T> handle) {
    return handle.get();
  }

 private:
 private:
  std::unordered_map<std::type_index, std::shared_ptr<void>> storage_;
};

void shout() {
  AssetManager asset_manager;

  auto a = asset_manager.load<Texture>(Texture());
}
template <typename T>
class Asset {
 public:
  bool loaded() { return data_ != nullptr; }

 private:
  std::unique_ptr<T> data_;
};
}  // namespace core