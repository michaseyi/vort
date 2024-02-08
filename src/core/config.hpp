#pragma once
#include <memory>
#include <string>

class Config {
public:
    const std::string shaderPrefixPath = "assets/shaders/";
    const std::string texturePreficPath = "assets/textures/";
    const bool headless = false;

    static Config& get() {
        if (!mInstance) {
            mInstance.reset(new Config());
        }
        return *mInstance;
    }

private:
    static inline std::unique_ptr<Config> mInstance;
};