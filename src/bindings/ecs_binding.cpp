#if defined(EMSCRIPTEN)
#include "ecs_binding.hpp"

#include "src/core/camera_controller.hpp"
#include "src/core/components.hpp"
#include "src/renderer/materials/basic_material.hpp"

EM_JS(void, _getMainCanvasSize, (int32_t * width, int32_t *height), {
    const canvas = document.querySelector("canvas.emscripten");
    const canvasWidth = new Int32Array([canvas.width]);
    const canvasHeight = new Int32Array([canvas.height]);
    HEAP32.set(canvasWidth, width / 4 | 0);
    HEAP32.set(canvasHeight, height / 4 | 0);
});

std::vector<EntityID> ECSBinding::entityGetChildren(EntityID entityID) {
    return mWorld->getChildren(entityID);
}

EntityID ECSBinding::entitiesCreateEntity(EntityID parentID) {
    return mWorld->newEntity("Entity0", EntityInterface::None, parentID);
}

void ECSBinding::entitiesRemoveEntity(EntityID entityID) {
    mWorld->removeEntity(entityID);
}

math::ivec2 ECSBinding::getMainWindowCanvasSize() {
    math::ivec2 canvasSize;
    _getMainCanvasSize(&canvasSize.x, &canvasSize.y);
    return canvasSize;
}

/**
 * @brief It is assumed that the world would have an address that remains constant for the entire runtime of the app.
 *
 */
void ECSBinding::setWorld(World &world) {
    mWorld = &world;
}

EntityID ECSBinding::entitiesCreateCubeMesh(EntityID parentID) {
    auto newEntityId = mWorld->newEntity("Cube0", EntityInterface::Mesh, parentID);

    mWorld->setComponents(newEntityId, Position(0.0f), Scale(1.0f), Orientation(),
                          Mesh(new Cube(1.0f), new BasicMaterial(BasicMaterialProps{.color = {0.5f, 0.5f, 0.5f}})));

    return newEntityId;
}
EntityID ECSBinding::entitiesCreateCylinderMesh(EntityID parentID) {
    auto newEntityId = mWorld->newEntity("Cylinder0", EntityInterface::Mesh, parentID);

    mWorld->setComponents(newEntityId, Position(0.0f), Scale(1.0f), Orientation(),
                          Mesh(new Cylinder(1.0f, 1.0f, 30), new BasicMaterial(BasicMaterialProps{.color = {0.5f, 0.5f, 0.5f}})));
    return newEntityId;
}

EntityID ECSBinding::entitiesCreateConeMesh(EntityID parentID) {
    auto newEntityId = mWorld->newEntity("Cone0", EntityInterface::Mesh, parentID);

    mWorld->setComponents(
        newEntityId, Position(0.0f), Scale(1.0f), Orientation(),
        Mesh(new Cone(1.0f, 3.0f, 30, 0.0f), new BasicMaterial(BasicMaterialProps{.color = {0.5f, 0.5f, 0.5f}})));
    return newEntityId;
}

EntityID ECSBinding::entitiesCreateUVSphereMesh(EntityID parentID) {
    auto newEntityId = mWorld->newEntity("UVSphere0", EntityInterface::Mesh, parentID);

    mWorld->setComponents(newEntityId, Position(0.0f), Scale(1.0f), Orientation(),
                          Mesh(new UVSphere(40, 20, 1.0f), new BasicMaterial(BasicMaterialProps{.color = {0.5f, 0.5f, 0.5f}})));
    return newEntityId;
}

EntityID ECSBinding::entitiesCreatePlaneMesh(EntityID parentID) {
    auto newEntityId = mWorld->newEntity("Plane0", EntityInterface::Mesh, parentID);

    mWorld->setComponents(newEntityId, Position(0.0f), Scale(1.0f), Orientation(),
                          Mesh(new Plane(10.0f, 10.0f), new BasicMaterial(BasicMaterialProps{.color = {0.5f, 0.5f, 0.5f}})));
    return newEntityId;
}

void ECSBinding::meshShadeNormal(EntityID entityID) {
    auto [mesh] = mWorld->getComponents<Mesh>(entityID);
    mesh.shadeNormal();
}

void ECSBinding::meshShadeSmooth(EntityID entityID) {
    auto [mesh] = mWorld->getComponents<Mesh>(entityID);
    mesh.shadeSmooth();
}

uint32_t ECSBinding::entityGetInterface(EntityID entityID) {
    return static_cast<uint32_t>(mWorld->getInterface(entityID));
}

uintptr_t ECSBinding::entityGetPosition(EntityID entityID) {
    auto [position] = mWorld->getComponents<Position>(entityID);
    return reinterpret_cast<uintptr_t>(&position);
}

uintptr_t ECSBinding::entityGetScale(EntityID entityID) {
    auto [scale] = mWorld->getComponents<Scale>(entityID);
    return reinterpret_cast<uintptr_t>(&scale);
}
uintptr_t ECSBinding::entityGetOrientation(EntityID entityID) {
    auto [orientation] = mWorld->getComponents<Orientation>(entityID);
    return reinterpret_cast<uintptr_t>(&orientation);
}

uintptr_t ECSBinding::entityGetName(EntityID entityID) {
    auto &name = mWorld->getName(entityID);

    return reinterpret_cast<uintptr_t>(name.c_str());
}

void ECSBinding::editorCameraZoom(float zoomChange) {
    auto [editorCameraController] = mWorld->getGlobal<CameraController>();
    editorCameraController.zoom(*mWorld, zoomChange);
}

void ECSBinding::editorCameraRotate(float pitchAngle, float yawAngle) {
    auto [editorCameraController] = mWorld->getGlobal<CameraController>();
    editorCameraController.rotate(*mWorld, pitchAngle, yawAngle);
}
EMSCRIPTEN_BINDINGS(Vort) {
    emscripten::function("entityGetChildren", ECSBinding::entityGetChildren);
    emscripten::function("entitiesRemoveEntity", ECSBinding::entitiesRemoveEntity);
    emscripten::function("entitiesCreateEntity", ECSBinding::entitiesCreateEntity);
    emscripten::function("entitiesCreateCubeMesh", ECSBinding::entitiesCreateCubeMesh);
    emscripten::function("entitiesCreateConeMesh", ECSBinding::entitiesCreateConeMesh);
    emscripten::function("entitiesCreateCylinderMesh", ECSBinding::entitiesCreateCylinderMesh);
    emscripten::function("entitiesCreatePlaneMesh", ECSBinding::entitiesCreatePlaneMesh);
    emscripten::function("entitiesCreateUVSphereMesh", ECSBinding::entitiesCreateUVSphereMesh);
    emscripten::function("entityGetPosition", ECSBinding::entityGetPosition);
    emscripten::function("entityGetOrientation", ECSBinding::entityGetOrientation);
    emscripten::function("entityGetScale", ECSBinding::entityGetScale);
    emscripten::function("entityGetInterface", ECSBinding::entityGetInterface);
    emscripten::function("entityGetName", ECSBinding::entityGetName);
    emscripten::function("meshShadeSmooth", ECSBinding::meshShadeSmooth);
    emscripten::function("meshShadeNormal", ECSBinding::meshShadeNormal);
    emscripten::function("editorCameraZoom", ECSBinding::editorCameraZoom);
    emscripten::function("editorCameraRotate", ECSBinding::editorCameraRotate);
    emscripten::register_vector<EntityID>("vector<EntityID>");
}

#endif