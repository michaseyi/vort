#pragma once
#include "components.hpp"
#include "data_structs.hpp"
#include "geometry.hpp"
#include "mesh.hpp"
#include "src/ecs/ecs.hpp"
#include "src/renderer/material.hpp"
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

    void operator()( World& tWorld) {
        auto window = WindowBuilder().clientAPI(ClientAPI::None).size(800, 800).build();

        assert(window);

        auto wgpuInitStatus = WGPUContext::init(&window);

        assert(wgpuInitStatus);

        ResourceGroup resourceGroupZero{};
        std::shared_ptr<Uniform<CommonUniforms>> commonUniforms = std::make_shared<Uniform<CommonUniforms>>(false);
        std::shared_ptr<Uniform<ModelUniforms>> modelUniforms = std::make_shared<Uniform<ModelUniforms>>(true);

        resourceGroupZero.setBindingCount(2);
        resourceGroupZero.bindResource(0, commonUniforms);
        resourceGroupZero.bindResource(1, modelUniforms);

        tWorld.setGlobal(RenderPass{}, std::move(commonUniforms), std::move(modelUniforms), std::move(resourceGroupZero));

        // set mesh component init and deint handler
        tWorld.setComponentInitHandler<Mesh>([](World& world, EntityID entityID) {
            auto [modelUniforms] = world.getGlobal<std::shared_ptr<Uniform<ModelUniforms>>>();
            modelUniforms->increment();

            std::cout << entityID << std::endl;
        });

        tWorld.setComponentDeinitHandler<Mesh>([](World& world, EntityID entityID) {
            auto [modelUniforms] = world.getGlobal<std::shared_ptr<Uniform<ModelUniforms>>>();
            modelUniforms->decrement();

            std::cout << entityID << std::endl;
        });

        // camera
        tWorld.setComponents<Position, Scale, Orientation, Mesh, CameraSettings>(
            tWorld.newEntity(), math::vec3(2.0f, 2.0f, -5.0f), math::vec3(1.0f), math::quat(),
            Mesh(new Cube(1.0f), new PhysicsMaterial(PhysicsMaterialProps{
                                     .color = {1.0f, 0.0f, 0.0f}, .reflectivity = 2.0f, .roughness = 1.0f, .shininess = 30.0f})),
            CameraSettings(math::radians(40.0f), window.width() / window.height(), 0.1f, 100.0f));

        // ambient light
        tWorld.setComponents(tWorld.newEntity(), AmbientLight(math::vec3(1.0f), 0.3));

        // point light
        tWorld.setComponents(tWorld.newEntity(), PointLight(math::vec3(1.0f), 0.5, math::vec3(1.0f), math::vec3(1.0f)));

        tWorld.setComponents<Position, Scale, Orientation, Mesh>(
            tWorld.newEntity(), math::vec3(0.0f), math::vec3(1.0f), math::quat(),
            Mesh(new UVSphere(20, 20, 1.0f),
                 new PhysicsMaterial(PhysicsMaterialProps{
                     .color = {1.0f, 0.0f, 0.0f}, .reflectivity = 2.0f, .roughness = 1.0f, .shininess = 30.0f})));

        tWorld.setComponents<Position, Scale, Orientation, Mesh>(
            tWorld.newEntity(), math::vec3(4.0f, 0.0f, 3.0f), math::vec3(1.0f), math::quat(),
            Mesh(new Cube(1.0f), new PhysicsMaterial(PhysicsMaterialProps{
                                     .color = {1.0f, 0.0f, 0.0f}, .reflectivity = 2.0f, .roughness = 1.0f, .shininess = 30.0f})));

        tWorld.addSystems(SystemSchedule::Update, windowUpdate, renderSystem);

        tWorld.setGlobal(std::move(window));
    }
};
