#pragma once
#include <chrono>
#include <cstdint>

namespace core {
class Timer {
 public:
  Timer() = default;
  ~Timer() = default;

  void start();

  void reset();

  void pause();

  void play();

  uint64_t elapsed_ms();

  uint64_t elapsed_us();

  uint64_t elapsed_s();

 private:
  std::chrono::time_point<std::chrono::high_resolution_clock> start_;
  std::chrono::time_point<std::chrono::high_resolution_clock> end_;
  std::chrono::time_point<std::chrono::high_resolution_clock> pause_start_;
  bool paused_;
};
}  // namespace core