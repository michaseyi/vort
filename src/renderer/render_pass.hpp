#pragma once
#include <iostream>
#include <memory>

#include "resource.hpp"
#include "src/core/components.hpp"
#include "src/core/data_structs.hpp"
#include "src/core/window.hpp"
#include "src/ecs/ecs.hpp"
#include "src/utils/raii.hpp"
#include "wgpu_context.hpp"

struct RenderTarget {
    std::vector<wgpu::ColorTargetState> colorTargets;
    std::vector<wgpu::RenderPassColorAttachment> colorAttachments;
    std::vector<RAIIWrapper<wgpu::TextureView>> textureViews;
};
struct Pass {
    virtual void operator()(wgpu::CommandEncoder &tCommandEncoder, Command &tCommand) = 0;

    virtual RenderTarget renderTargets() = 0;

    void addSubPass(std::shared_ptr<Pass> tPass) {
        subPasses.push_back(std::move(tPass));
    };

    std::vector<std::shared_ptr<Pass>> subPasses;

    virtual ~Pass() = default;
};

struct DefaultRenderPass : public Pass {
    RenderTarget renderTargets() {
        RenderTarget renderTarget;

        auto &context = WGPUContext::getContext();
        auto swapChain = context.getSwapChain();

        renderTarget.textureViews.push_back(RAIIWrapper<wgpu::TextureView>(swapChain.getCurrentTextureView()));
        renderTarget.textureViews.push_back(context.getMultiSampleTextureView());

        wgpu::RenderPassColorAttachment colorAttachment = wgpu::Default;
        colorAttachment.clearValue = {1, 0, 0, 1};
        colorAttachment.loadOp = wgpu::LoadOp::Clear;
        colorAttachment.storeOp = wgpu::StoreOp::Store;
        colorAttachment.view = *renderTarget.textureViews[1];
        colorAttachment.resolveTarget = *renderTarget.textureViews[0];

        wgpu::BlendComponent blendComponent = wgpu::Default;
        blendComponent.srcFactor = wgpu::BlendFactor::SrcAlpha;
        blendComponent.operation = wgpu::BlendOperation::Add;
        blendComponent.dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha;

        wgpu::BlendState colorTargetBlendState = wgpu::Default;
        colorTargetBlendState.alpha = blendComponent;
        colorTargetBlendState.color = blendComponent;

        wgpu::ColorTargetState colorTarget = wgpu::Default;
        colorTarget.format = context.getOutputTextureFormat();
        colorTarget.writeMask = wgpu::ColorWriteMask::All;

        // TODO: Invalid code; attempts to return address of local variable;
        
        colorTarget.blend = &colorTargetBlendState;

        // color attachments.
        renderTarget.colorAttachments = {colorAttachment};

        // color targets.
        renderTarget.colorTargets = {colorTarget};

        return renderTarget;
    }

    void operator()(wgpu::CommandEncoder &tCommandEncoder, Command &tCommand) {
        auto &context = WGPUContext::getContext();
        auto device = context.getDevice();

        auto depthStencilTextureView = context.getDepthStencilTextureView();

        auto [window, commonUniforms] = tCommand.getGlobal<Window, std::shared_ptr<Uniform<CommonUniforms>>>();

        auto renderTarget = renderTargets();

        wgpu::RenderPassDescriptor renderPassDesc = wgpu::Default;

        renderPassDesc.colorAttachmentCount = renderTarget.colorAttachments.size();
        renderPassDesc.colorAttachments = renderTarget.colorAttachments.data();

        wgpu::RenderPassDepthStencilAttachment depthStencilAttachment = wgpu::Default;
        depthStencilAttachment.depthClearValue = 1;
        depthStencilAttachment.depthLoadOp = wgpu::LoadOp::Clear;
        depthStencilAttachment.depthStoreOp = wgpu::StoreOp::Store;
        depthStencilAttachment.stencilLoadOp = wgpu::LoadOp::Clear;
        depthStencilAttachment.stencilStoreOp = wgpu::StoreOp::Store;
        depthStencilAttachment.stencilClearValue = 1;
        depthStencilAttachment.view = *depthStencilTextureView;

        renderPassDesc.depthStencilAttachment = &depthStencilAttachment;

        RAIIWrapper<wgpu::RenderPassEncoder> renderPassEncoder = tCommandEncoder.beginRenderPass(renderPassDesc);

#pragma region setting common uniforms

        auto [cameraSettings, cameraPosition, cameraOrientation] =
            *tCommand.query<Columns<CameraSettings, Position, Orientation>, Tags<>>().begin();

        math::mat4 viewMatrix = math::lookAt(math::vec3(), math::vec3(), math::vec3());

        math::mat4 projectionMatrix =
            math::perspectiveZO(cameraSettings.fov, cameraSettings.aspectRatio, cameraSettings.near, cameraSettings.far);

        CommonUniforms common{};
        common.view_projection_matrix = projectionMatrix * viewMatrix;
        common.resolution = math::uvec2(window.width(), window.height());

        // TODO: make a global time component and pull common.time from there
        common.time = glfwGetTime();

        for (auto [spotLight] : tCommand.query<Columns<SpotLight>, Tags<>>()) {
            assert(common.spot_light_count < 5 && "Maxiimum of 5 spotlight");
            common.spot_lights[common.spot_light_count] = spotLight;
            common.spot_light_count++;
        }
        for (auto [pointLight] : tCommand.query<Columns<PointLight>, Tags<>>()) {
            assert(common.point_light_count < 5 && "Maxiimum of 5 pointLight");
            common.point_lights[common.point_light_count] = pointLight;
            common.point_light_count++;
        }
        for (auto [directionalLight] : tCommand.query<Columns<DirectionalLight>, Tags<>>()) {
            assert(common.directional_light_count < 5 && "Maxiimum of 5 directionalLight");
            common.directional_lights[common.directional_light_count] = directionalLight;
            common.directional_light_count++;
        }
        for (auto [ambientLight] : tCommand.query<Columns<AmbientLight>, Tags<>>()) {
            assert(common.ambient_light_count < 5 && "Maxiimum of 5 ambientLight");
            common.ambient_lights[common.ambient_light_count] = ambientLight;
            common.ambient_light_count++;
        }

        commonUniforms->set(common);

#pragma endregion

#pragma region drawing each mesh
//         for (auto [position, scale, orientation, mesh] :
//              tCommand.query<Columns<Position, Scale, Orientation, Mesh>, Tags<>>()) {
//             mesh.updateGPUMesh();

//             math::mat4 modelMatrix = math::translate(math::mat4(1.0f), math::vec3(position));
//             modelMatrix = math::rotate(modelMatrix, math::angle(math::quat(orientation)),
//                                        math::axis(math::quat(orientation)));
//             modelMatrix = math::scale(modelMatrix, math::vec3(scale));

//             auto normalMatrix = math::mat3(modelMatrix);

//             ModelUniforms model{};
//             model.model_matrix = modelMatrix;
//             model.normal_matrix = normalMatrix;

//             wgpu::RenderPipelineDescriptor renderPipelineDesc = wgpu::Default;

//             // drawing modes modes -> surfaces (the default), edges(wireframe), vertices
//             auto meshDrawData = mesh.drawData();

//             renderPipelineDesc.vertex.bufferCount = meshDrawData.vertexBuffers.size();
//             renderPipelineDesc.vertex.buffers = meshDrawData.vertexBufferLayouts.data();
//             renderPipelineDesc.vertex.entryPoint = "vs_main";
//             // renderPipelineDesc.vertex.module

//             renderPipelineDesc.multisample.count = 4;
//             renderPipelineDesc.multisample.alphaToCoverageEnabled = true;
//             renderPipelineDesc.multisample.mask = ~0u;

//             wgpu::FragmentState fragmentState = wgpu::Default;
//             fragmentState.targets = renderTarget.colorTargets.data();
//             fragmentState.targetCount = renderTarget.colorTargets.size();
//             fragmentState.entryPoint = "fs_main";
//             // fragmentState.module
//             renderPipelineDesc.fragment = &fragmentState;

//             renderPipelineDesc.primitive.cullMode = wgpu::CullMode::Back;
//             renderPipelineDesc.primitive.frontFace = wgpu::FrontFace::CCW;
//             renderPipelineDesc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;

// #pragma region pipeline for resources
//             wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor = wgpu::Default;

//             RAIIWrapper<wgpu::PipelineLayout> layout =
//             device.createPipelineLayout(pipelineLayoutDescriptor);
// #pragma endregion
//             renderPipelineDesc.layout = *layout;

//             RAIIWrapper<wgpu::RenderPipeline> renderPipeline =
//                 device.createRenderPipeline(renderPipelineDesc);

//             // renderPassEncoder->setBindGroup()
//             for (uint32_t i = 0; i < meshDrawData.vertexBuffers.size(); i++) {
//                 renderPassEncoder->setVertexBuffer(
//                     i, meshDrawData.vertexBuffers[i], 0,
//                     meshDrawData.vertexBufferLayouts[i].arrayStride * meshDrawData.vertexCount);
//             }
//             renderPassEncoder->setPipeline(*renderPipeline);
//             renderPassEncoder->draw(meshDrawData.vertexCount, 0, 1, 1);
//         }
#pragma endregion

        renderPassEncoder->end();
    };
};

struct RenderPass {
    RenderPass() : rootPass(std::make_shared<DefaultRenderPass>()) {
    }

    void render(Command &tCommand) {
        auto &context = WGPUContext::getContext();
        auto device = context.getDevice();
        auto queue = context.getQueue();

        wgpu::CommandEncoderDescriptor commandEncoderDesc = wgpu::Default;
        RAIIWrapper<wgpu::CommandEncoder> commandEncoder = device.createCommandEncoder(commandEncoderDesc);

        if (rootPass) {
            (*rootPass)(*commandEncoder, tCommand);
        }

        wgpu::CommandBufferDescriptor commandBufferDesc = wgpu::Default;
        RAIIWrapper<wgpu::CommandBuffer> commandBuffer = commandEncoder->finish(commandBufferDesc);
        queue.submit(*commandBuffer);

#if !defined(EMSCRIPTEN)
        context.getSwapChain().present();
#endif
    }

private:
    std::shared_ptr<Pass> rootPass;
};
