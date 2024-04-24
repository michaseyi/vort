#pragma once
#include <cstdint>
#include <memory>

struct Image {
  int32_t width = 0;
  int32_t height = 0;
  int32_t channel_count = 0;
  std::unique_ptr<uint8_t> buffer;
};