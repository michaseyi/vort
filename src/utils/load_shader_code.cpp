#include <string>

#include "src/core/config.hpp"
#include "utils.hpp"

std::string load_shader_code(std::string tPath) {
  auto& config = core::Config::get();
  return load_string(config.shader_prefix_path + tPath);
}