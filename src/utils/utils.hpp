#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "image.hpp"
#include "raii.hpp"

extern std::string loadString(std::string);

extern std::vector<uint8_t> loadBuffer(std::string);

extern Image loadImage(std::string);

extern std::string preprocessShader(std::string &);

extern std::string loadShaderCode(std::string);