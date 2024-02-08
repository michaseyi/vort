#include <string>

#include "src/core/config.hpp"
#include "utils.hpp"

std::string loadShaderCode(std::string tPath) {
    auto &config = Config::get();
    return loadString(config.shaderPrefixPath + tPath);
}