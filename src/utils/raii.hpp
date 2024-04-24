#pragma once
#include <iostream>
#include <typeinfo>

template <typename T>
struct DefaultRaiiDeleter {
  static void deleter(T& t) { t.release(); }
};

template <typename T>
struct RaiiDeleter : DefaultRaiiDeleter<T> {};

template <typename T>
class RaiiWrapper {
 public:
  RaiiWrapper() = default;
  RaiiWrapper(T tRaw) : raw_(tRaw){};

  RaiiWrapper(std::nullptr_t) : raw_(nullptr){};

  RaiiWrapper& operator=(RaiiWrapper&& other) {
    if (raw_) {
      RaiiDeleter<T>::deleter(raw_);
    }
    raw_ = other.raw_;
    other.raw_ = nullptr;
    return *this;
  }
  RaiiWrapper(RaiiWrapper&& other) {
    raw_ = other.raw_;
    other.raw_ = nullptr;
  }

  RaiiWrapper& operator=(const RaiiWrapper& other) = delete;

  RaiiWrapper(const RaiiWrapper& other) = delete;

  auto operator->() {
    if constexpr (std::is_pointer_v<T>) {
      return raw_;
    } else {
      return &raw_;
    }
  }

  T& operator*() { return raw_; }

  const T& operator*() const { return raw_; }

  ~RaiiWrapper() {
    if (raw_) {
      RaiiDeleter<T>::deleter(raw_);
    }
  }

  operator T() { return raw_; }

 private:
  T raw_ = nullptr;
};