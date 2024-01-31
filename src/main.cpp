#include "core/plugins.hpp"
#include "utils/utils.hpp"

int32_t main() {
    auto steel = loadImage("assets/textures/steel.jpg");
    World().addPlugin(DefaultPlugin{}).run();
    return 0;
}
