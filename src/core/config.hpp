#pragma once
#include <memory>
#include <string>

namespace core {

class Config {
 public:
  std::string shader_prefix_path = "assets/shaders/";
  std::string texture_prefix_path = "assets/textures/";
  bool headless = false;

  static Config& get() {
    if (!instance_) {
      instance_.reset(new Config());
    }
    return *instance_;
  }

  static void init(int32_t argc, char** argv) {
    get();
    for (int32_t i = 0; i < argc; i++) {
      if (std::string(argv[i]) == "--headless") {
        get().headless = true;
      }
    }
  }

 private:
  static inline std::unique_ptr<Config> instance_;
};
}  // namespace core