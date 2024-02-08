#include "core/plugins.hpp"

int32_t main() {
    World().addPlugin(DefaultPlugin()).run();

    return 0;
}
