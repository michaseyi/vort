#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "image.hpp"
#include "raii.hpp"

extern std::string load_string(std::string);

extern std::vector<uint8_t> load_buffer(std::string);

extern Image load_image(std::string);

extern std::string preprocess_shader(std::string);

extern std::string load_shader_code(std::string);