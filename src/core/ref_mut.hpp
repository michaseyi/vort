#include <atomic>
#include <vector>

template <typename... ComponentT>
class RefMut {
 public:
  std::tuple<ComponentT&...> operator*() {
    return std::make_tuple(*std::get<ComponentT>(components_)...);
  }
  RefMut(std::tuple<ComponentT*...> components, std::atomic<int32_t>* counter)
      : components_(components), counter_(counter) {
    int32_t expected = 0;
    
    while (!counter_->compare_exchange_weak(expected, -1)) {
      expected = 0;
      counter_->wait(*counter_);
    };
  }

  ~RefMut() {
    if (counter_) {
      *(counter_) = 0;
      counter_->notify_all();
    }
  }

 private:
  std::tuple<ComponentT*...> components_ = {};
  std::atomic<int32_t>* counter_ = nullptr;
};
