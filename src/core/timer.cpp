#include "timer.hpp"

#include <chrono>
#include <cstdint>

namespace core {
void Timer::start() {
  start_ = std::chrono::high_resolution_clock::now();
};

void Timer::reset() {
  start_ = std::chrono::high_resolution_clock::now();
  paused_ = false;
}

void Timer::pause() {
  if (paused_) {
    return;
  }
  paused_ = true;
  pause_start_ = std::chrono::high_resolution_clock::now();
}
void Timer::play() {
  if (!paused_) {
    return;
  }
  paused_ = false;
  start_ = std::chrono::high_resolution_clock::now();
}

uint64_t Timer::elapsed_ms() {
  end_ = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::milliseconds>(end_ - start_)
      .count();
}

uint64_t Timer::elapsed_us() {
  end_ = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(end_ - start_)
      .count();
}

uint64_t Timer::elapsed_s() {
  end_ = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::seconds>(end_ - start_)
      .count();
}
}  // namespace core