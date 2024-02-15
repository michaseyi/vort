#pragma once
#include "src/ecs/entities.hpp"
#include "src/ecs/query.hpp"
#include "src/math/ray_intersections.hpp"

class Events {};

using MouseDownCallback = void (*)(World&, EntityID);
struct InterractionEventCallbacks {
    MouseDownCallback onMouseDown = nullptr;
    MouseDownCallback onMouseUp = nullptr;
    MouseDownCallback onHoverStart = nullptr;
    MouseDownCallback onHoverStop = nullptr;
};

// onMouseDown
// onMouseUp
// onHoverStart
// onHoverEnd

using None = Tags<>;


void shout(World& tWorld) {
    auto query = tWorld.query<Columns<InterractionEventCallbacks>, Tags<>>();

    for (auto i = query.begin(); i != query.end(); i++) {
        auto [eventCallbacks] = *i;

        if (eventCallbacks.onHoverStart != nullptr) {
            continue;
        }

        auto entityId = i.currentEntityID();

        eventCallbacks.onHoverStart(tWorld, entityId);
    }
}
