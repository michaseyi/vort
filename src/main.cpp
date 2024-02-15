#include "core/editor_plugin.hpp"

int32_t main() {
    World().addPlugin(EditorPlugin()).run();

    return 0;
}
