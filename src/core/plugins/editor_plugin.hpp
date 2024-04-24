#pragma once

#include "../render_target_outputs.hpp"
#include "src/ecs/ecs.hpp"
namespace core::plugins {
struct EditorPlugin {
  static void render_system(ecs::Command& command,
                            ecs::Global<RenderTargetOutputs>& global);

  void operator()(ecs::World& world);
};
}  // namespace core::plugins
