#include "core/config.hpp"
#include "core/plugins/editor_plugin.hpp"

int32_t main(int32_t argc, char** argv) {
  core::Config::init(argc, argv);
  ecs::World().add_plugins(core::plugins::EditorPlugin()).run();
  return 0;
}