#pragma once
#include "components.hpp"
#include "geometry.hpp"
#include "mesh.hpp"
#include "src/ecs/ecs.hpp"
#include "src/renderer/render_pass.hpp"
#include "src/renderer/wgpu_context.hpp"
#include "window.hpp"

struct DefaultPlugin {
    static void renderSystem(Command& command, Global<RenderPass>& global) {
        auto& context = WGPUContext::getContext();

        auto [renderPass] = global;

#if !defined(EMSCRIPTEN)
        context.getDevice().tick();
#endif

        wgpu::CommandEncoderDescriptor commandEncoderDesc = wgpu::Default;
        RAIIWrapper<wgpu::CommandEncoder> commandEncoder =
            context.getDevice().createCommandEncoder(commandEncoderDesc);

        renderPass(*commandEncoder, command);

        wgpu::CommandBufferDescriptor commandBufferDesc = wgpu::Default;
        RAIIWrapper<wgpu::CommandBuffer> commandBuffer = commandEncoder->finish(commandBufferDesc);
        context.getQueue().submit(*commandBuffer);

#if !defined(EMSCRIPTEN)
        context.getSwapChain().present();
#endif
    }

    static void windowUpdate(Global<Window, AppState>& global) {
        auto [window, appState] = global;
        window.processEvents();

        if (!window.isOpen()) {
            appState.running = false;
        }
    }

    void operator()(World& tWorld) {
        auto window = WindowBuilder().clientAPI(ClientAPI::None).size(800, 800).build();

        assert(window);

        auto wgpuInitStatus = WGPUContext::init(&window);

        assert(wgpuInitStatus);

        tWorld.setComponents(tWorld.newEntity(), Mesh{UVSphere{20, 20, 1.0f}}, Position{math::vec3{0.0f}},
                             Scale(math::vec3(1.0f)));

        tWorld.addSystems(SystemSchedule::Update, windowUpdate, renderSystem);
        tWorld.setGlobal(std::move(window));
        tWorld.setGlobal(RenderPass{});
    }
};
