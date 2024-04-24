#pragma once
#include <cstdint>
#include <exception>
#include <iostream>
#include <unordered_map>
#include <vector>
namespace ecs {

template <typename K, typename V>
class ArrayHashMap {
 public:
  template <typename _K, typename _V>
  uint32_t put(_K&& key, _V value) {
    return put_impl(std::forward<_K&>(key), std::move(value));
  }

  void reserve(size_t size) {
    storage_.reserve(size);
    index_map_.reserve(size);
  }

  // Note the data returned is valid as long as the underlyning datastructure has not been resized.
  V* get(const K& key) {
    if (auto index_iter = index_map_.find(key);
        index_iter != index_map_.end()) {
      return &storage_[index_iter->second];
    }
    return nullptr;
  }

  std::vector<V>& values() { return storage_; }

  inline uint32_t get_dense_storage_index(K key) { return index_map_[key]; }

 private:
  uint32_t put_impl(const K& key, V value) {
    if (auto index_iter = index_map_.find(key); index_iter != index_map_.end()) {
      storage_[index_iter->second] = std::move(value);
      return index_iter->second;
    } else {
      size_t index_for_new_value = storage_.size();
      index_map_[key] = index_for_new_value;
      storage_.emplace_back(std::move(value));
      return index_for_new_value;
    }
  }

  std::vector<V> storage_;
  std::unordered_map<K, uint32_t> index_map_;
};
}  // namespace ecs