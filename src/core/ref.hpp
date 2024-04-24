#include <atomic>
#include <vector>

template <typename... ComponentT>
class Ref {
 public:
  std::tuple<const ComponentT&...> operator*() {
    return std::make_tuple(*std::get<ComponentT*>(components_)...);
  }

  Ref(std::tuple<ComponentT*...> components, std::atomic<int32_t>* counter)
      : components_(components), counter_(counter) {
    while (*counter_ == -1) {
      counter_->wait(*counter_);
    }
    *(counter_)++;
  }

  ~Ref() {
    if (counter_) {
      *(counter_)--;
      counter_->notify_one();
    }
  }

 private:
  std::tuple<ComponentT*...> components_ = {};
  std::atomic<int32_t>* counter_ = nullptr;
};
