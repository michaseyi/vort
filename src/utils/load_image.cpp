
#include <cassert>
#include <string>

#include "image.hpp"
#if !defined(EMSCRIPTEN)
#define STB_IMAGE_IMPLEMENTATION
#endif
#include "stb_image.h"
#include "utils.hpp"

Image loadImage(std::string tPath) {
    std::vector<uint8_t> buffer = loadBuffer(tPath);

    Image image;

    image.buffer.reset(stbi_load_from_memory(buffer.data(), buffer.size(), &image.width, &image.height,
                                             &image.channelCount, 0));

    assert(image.buffer && "Error decoding image");

    return image;
}