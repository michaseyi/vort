
#include <cassert>
#include <string>

#include "image.hpp"
#if !defined(EMSCRIPTEN)
#define STB_IMAGE_IMPLEMENTATION
#endif
#include "stb_image.h"
#include "utils.hpp"

Image load_image(std::string tPath) {
  std::vector<uint8_t> buffer = load_buffer(tPath);

  Image image;

  image.buffer.reset(stbi_load_from_memory(buffer.data(), buffer.size(),
                                           &image.width, &image.height,
                                           &image.channel_count, 4));

  assert(image.buffer && "Error decoding image");

  return image;
}