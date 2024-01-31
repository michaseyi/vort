#pragma once
#include <iostream>
#include <memory>

#include "src/ecs/ecs.hpp"
#include "src/utils/raii.hpp"
#include "wgpu_context.hpp"

struct SubPass {
    // input
    void operator()(wgpu::CommandEncoder &tCommandEncoder, Command &) {
        wgpu::RenderPassDescriptor renderPassDesc = wgpu::Default;

        RAIIWrapper<wgpu::RenderPassEncoder> renderPassEncoder =
            tCommandEncoder.beginRenderPass(renderPassDesc);

        renderPassEncoder->end();
    }

    std::vector<std::unique_ptr<SubPass>> dependencies = {};
};

struct RenderPass {
    void callPass(SubPass &tPass, wgpu::CommandEncoder &tCommandEncoder, Command &tCommand) {
        std::cout << tPass.dependencies.size() << std::endl;
        for (auto &subPass : tPass.dependencies) {
            if (subPass) {
                callPass(*subPass, tCommandEncoder, tCommand);
            }
        }

        tPass(tCommandEncoder, tCommand);
    }

    void operator()(wgpu::CommandEncoder &tCommandEncoder, Command &tCommand) {
        if (rootPass) {
            callPass(*rootPass, tCommandEncoder, tCommand);
        }
    }

    std::unique_ptr<SubPass> rootPass;
};
