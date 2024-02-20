#pragma once

#if defined(EMSCRIPTEN)
#include <emscripten/bind.h>
#include <emscripten/emscripten.h>

#include "src/ecs/entities.hpp"
#include "src/math/math.hpp"

class ECSBinding {
public:
    static std::vector<EntityID> entityGetChildren(EntityID entityID);

    static EntityID entitiesCreateEntity(EntityID parentID);

    static void entitiesRemoveEntity(EntityID entityID);

    static math::ivec2 getMainWindowCanvasSize();

    // It is assumed the the world would have an address that remains constant for the entire runtime of the app.
    static void setWorld(World &world);

    static EntityID entitiesCreateUVSphereMesh(EntityID parentID);

    static EntityID entitiesCreateCubeMesh(EntityID parentID);

    static EntityID entitiesCreatePlaneMesh(EntityID parentID);

    static EntityID entitiesCreateCylinderMesh(EntityID parentID);

    static EntityID entitiesCreateConeMesh(EntityID parentID);

    static void meshShadeSmooth(EntityID entityID);

    static void meshShadeNormal(EntityID entityID);

    static uintptr_t entityGetPosition(EntityID entityID);

    static uintptr_t entityGetName(EntityID entityID);

    static uintptr_t entityGetScale(EntityID entityID);

    static uint32_t entityGetInterface(EntityID entityID);

    static uintptr_t entityGetOrientation(EntityID entityID);

    static void editorMoveCamera(float xChange, float yChange, float zChange);

    static void editorRotateCamera(float pitchAngle, float yawAngle);

    static uintptr_t globalGetAppState();

private:
    inline static World *mWorld;
};

#endif