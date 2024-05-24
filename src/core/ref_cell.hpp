#pragma once
#include <optional>
#include "src/math/math.hpp"

#define panic(message)                 \
  do {                                 \
    std::cerr << message << std::endl; \
    abort()                            \
  } while (0)

template <typename T>
class Ref {
 public:
  Ref(const T* inner) : inner(inner_) {}

  const T& operator*() const { return *inner_; }
  const T* operator->() const { return inner_; }

 private:
  const T* inner_;
};

template <typename T>
class RefMut {
 public:
  RefMut(T* inner) : inner(inner_) {}
  RefMut(const RefMut&) = delete;
  RefMut& operator=(const RefMut&) = delete;

  T& operator*() { return *inner_; };
  T* operator->() { return inner_; };

 private:
  T* inner_;
};

template <typename T>
class RefCell {
 public:
  RefCell(T&& inner) : inner_(inner){};
  RefMut<T> borrow_mut() {
    if (borrow_counter_ != 0) {
      panic("Invalid borrow_mut call");
    }
    borrow_counter_ = -1;

    return RefMut<T>(&inner_);
  };

  Ref<T> borrow() {
    if (borrow_counter_ == -1) {
      panic("Invalid borrow call");
    }

    borrow_counter_++;

    return Ref<T>(&inner_);
  };

  std::optional<Ref<T>> try_borrow() {
    if (borrow_counter_ == -1) {
      return std::nullopt;
    }

    borrow_counter_++;
    return Ref<T>(&inner_);
  }

  std::optional<RefMut<T>> borrow_mut() {
    if (counter_ != 0) {
      return std::nullopt;
    }

    counter_ = -1;
    return RefMut<T>(&inner_);
  };

 private:
  T inner_;
  int32_t borrow_counter_ = 0;
};

struct Transform {
  math::vec3 position;
  math::vec3 scale;
  math::quat orientation;
};

struct t {
  math::vec3 position;

  void shout() {}
};
void shout() {
  auto transform = RefCell(Transform());

  {
    auto ref_mut = transform.borrow_mut();
    (*ref_mut).position += 2.0f;
  }
}