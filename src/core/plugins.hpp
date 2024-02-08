#pragma once
#include "components.hpp"
#include "data_structs.hpp"
#include "geometry.hpp"
#include "mesh.hpp"
#include "src/bindings/ecs_binding.hpp"
#include "src/ecs/ecs.hpp"
#include "src/renderer/materials/basic_material.hpp"
#include "src/renderer/render_pass.hpp"
#include "src/renderer/texture.hpp"
#include "src/renderer/wgpu_context.hpp"
#include "window.hpp"

struct DefaultPlugin {
    static void renderSystem(Command& command, Global<RenderPass>& global) {
        auto [renderPass] = global;

#if !defined(EMSCRIPTEN)
        auto& context = WGPUContext::getContext();
        context.getDevice().tick();
#endif

        renderPass.render(command);
    }

    static void windowUpdate(Global<Window, AppState>& global) {
        auto [window, appState] = global;
        window.processEvents();

        if (!window.isOpen()) {
            appState.running = false;
        }
    }

    void operator()(World& tWorld) {
        int32_t width = 800, height = 800;
#if defined(EMSCRIPTEN)
        ECSBinding::setWorld(tWorld);

        auto canvasSize = ECSBinding::getMainWindowCanvasSize();
        width = canvasSize.x;
        height = canvasSize.y;
#endif
        auto window = WindowBuilder().clientAPI(ClientAPI::None).size(width, height).build();

        assert(window);

        auto wgpuInitStatus = WGPUContext::init(&window);

        assert(wgpuInitStatus);

        ResourceGroup resourceGroupZero{};
        std::shared_ptr<Uniform> commonUniforms = std::make_shared<Uniform>(sizeof(CommonUniforms), false);
        std::shared_ptr<Uniform> modelUniforms = std::make_shared<Uniform>(sizeof(ModelUniforms), true);

        resourceGroupZero.setBindingCount(2);
        resourceGroupZero.bindResource(0, commonUniforms);
        resourceGroupZero.bindResource(1, modelUniforms);

        tWorld.setGlobal(RenderPass{}, GeneralUniforms{std::move(commonUniforms), std::move(modelUniforms)},
                         std::move(resourceGroupZero));

        // set mesh component init and deint handler
        tWorld.setComponentInitHandler<Mesh>([](World& world, EntityID entityID) {
            auto [generalUniforms, resourceGroupZero] = world.getGlobal<GeneralUniforms, ResourceGroup>();
            generalUniforms.modelUniforms->increment();
            resourceGroupZero.bindResource(1, generalUniforms.modelUniforms);
        });

        tWorld.setComponentDeinitHandler<Mesh>([](World& world, EntityID entityID) {
            auto [generalUniforms, resourceGroupZero] = world.getGlobal<GeneralUniforms, ResourceGroup>();
            generalUniforms.modelUniforms->decrement();
            resourceGroupZero.bindResource(1, generalUniforms.modelUniforms);
        });

        // scene
        auto scene = tWorld.newEntity("Scene0", EntityInterface::Scene);

        // camera
        tWorld.setComponents(
            tWorld.newEntity("Camera0", EntityInterface::Camera, scene), Position(7.0f, 0.0f, 0.0f), Scale(1.0f), Orientation(),
            CameraSettings(45.0f, static_cast<float>(window.width()) / static_cast<float>(window.height()), 0.1f, 100.0f));

        // ambient light
        tWorld.setComponents(tWorld.newEntity("Ambient Light0", EntityInterface::Light, scene),
                             AmbientLight(math::vec3(1.0f), 0.2));

        tWorld.setComponents(
            tWorld.newEntity("Point Light0", EntityInterface::Light, scene),
            PointLight(math::vec3(0.0f, 8.0f, -10.0f), 0.8, math::vec3(1.0f), math::vec3(0.0007f, 0.0014f, 1.0f)));

        tWorld.setComponents(
            tWorld.newEntity("Point Light1", EntityInterface::Light, scene),
            PointLight(math::vec3(3.0f, 4.0f, -0.3f), 0.8, math::vec3(1.0f), math::vec3(0.0007f, 0.0014f, 1.0f)));

        tWorld.setComponents(tWorld.newEntity("Cube0", EntityInterface::Mesh, scene), Position(0.0f, 1.0f, 0.0f), Scale(1.0f),
                             Orientation(),
                             Mesh(new Cube(1.0f), new BasicMaterial(BasicMaterialProps{.color = {0.5f, 0.5f, 0.5f}})));

        tWorld.setComponents(tWorld.newEntity("Plane0", EntityInterface::Mesh, scene), Position(0.0f), Scale(1.0f), Orientation(),
                             Mesh(new Plane(20.0f, 20.0f), new BasicMaterial(BasicMaterialProps{.color = {0.5f, 0.5f, 0.5f}})));

        tWorld.addSystems(SystemSchedule::Update, windowUpdate, renderSystem);

        tWorld.setGlobal(std::move(window));
    }
};
