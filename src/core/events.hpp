#pragma once
#include "src/ecs/entities.hpp"
#include "src/ecs/query.hpp"

namespace core {

class Events {};

using MouseDownCallback = void (*)(ecs::World&, ecs::EntityId);
struct InterractionEventCallbacks {
  MouseDownCallback on_mosue_down = nullptr;
  MouseDownCallback on_mouse_up = nullptr;
  MouseDownCallback on_hover_start = nullptr;
  MouseDownCallback on_hover_stop = nullptr;
};

// onMouseDown
// onMouseUp
// onHoverStart
// onHoverEnd

}  // namespace core