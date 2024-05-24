
#include "render_pass.hpp"

#include <iostream>
#include <memory>

#include "html_canvas_render_target.hpp"
#include "pipeline.hpp"
#include "pipeline_entries.hpp"
#include "resource.hpp"
#include "shader.hpp"
#include "src/core/components.hpp"
#include "src/core/data_structs.hpp"
#include "src/core/window.hpp"
#include "src/ecs/ecs.hpp"
#include "src/utils/raii.hpp"
#include "wgpu_context.hpp"

namespace renderer {

void Pass::add_subpasses(std::shared_ptr<Pass> pass) {
  subpasses.push_back(std::move(pass));
};

RenderTarget DefaultRenderPass::render_targets(
    HtmlCanvasRenderTarget& html_render_target) {
  RenderTarget render_target;

  auto& context = WgpuContext::get();

  render_target.texture_views.push_back(RaiiWrapper<wgpu::TextureView>(
      html_render_target.get_current_texture_view()));

  if (html_render_target.multisample_count > 1) {
    render_target.texture_views.push_back(
        html_render_target.get_multisample_texture_view());
  }

  wgpu::RenderPassColorAttachment color_attachment = wgpu::Default;
  color_attachment.clearValue = {0.172, 0.172, 0.172, 1};
  color_attachment.loadOp = wgpu::LoadOp::Clear;
  color_attachment.storeOp = wgpu::StoreOp::Store;

  if (html_render_target.multisample_count > 1) {
    color_attachment.view = *render_target.texture_views[1];
    color_attachment.resolveTarget = *render_target.texture_views[0];
  } else {
    color_attachment.view = *render_target.texture_views[0];
  }

  wgpu::BlendComponent blend_component = wgpu::Default;
  blend_component.srcFactor = wgpu::BlendFactor::SrcAlpha;
  blend_component.operation = wgpu::BlendOperation::Add;
  blend_component.dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha;

  wgpu::BlendState color_target_blend_state = wgpu::Default;
  color_target_blend_state.alpha = blend_component;
  color_target_blend_state.color = blend_component;

  wgpu::ColorTargetState color_target = wgpu::Default;
  color_target.format = html_render_target.output_texture_format;
  color_target.writeMask = wgpu::ColorWriteMask::All;

  render_target.color_target_blend_state = {color_target_blend_state};

  color_target.blend = &render_target.color_target_blend_state[0];

  // color attachments.
  render_target.color_attachments = {color_attachment};

  // color targets.
  render_target.color_targets = {color_target};

  return render_target;
}

void DefaultRenderPass::operator()(wgpu::CommandEncoder& command_encoder,
                                   ecs::World& world,
                                   HtmlCanvasRenderTarget& html_render_target) {
  auto& context = WgpuContext::get();
  auto device = context.get_device();

  auto depth_stencil_texture_view =
      html_render_target.get_depth_stencil_texture_view();

  // auto [resourceGroupZero, generalUniforms] =
  //     tWorld.getGlobal<ResourceGroup, GeneralUniforms>();

  auto [general] = world.get_global<General>();

  auto render_target = render_targets(html_render_target);

  wgpu::RenderPassDescriptor render_pass_desc = wgpu::Default;

  render_pass_desc.colorAttachmentCount =
      render_target.color_attachments.size();
  render_pass_desc.colorAttachments = render_target.color_attachments.data();

  wgpu::RenderPassDepthStencilAttachment depth_stencil_attachment =
      wgpu::Default;
  depth_stencil_attachment.depthClearValue = 1;
  depth_stencil_attachment.depthLoadOp = wgpu::LoadOp::Clear;
  depth_stencil_attachment.depthStoreOp = wgpu::StoreOp::Store;
  depth_stencil_attachment.stencilLoadOp = wgpu::LoadOp::Clear;
  depth_stencil_attachment.stencilStoreOp = wgpu::StoreOp::Store;
  depth_stencil_attachment.stencilClearValue = 1;
  depth_stencil_attachment.view = *depth_stencil_texture_view;

  render_pass_desc.depthStencilAttachment = &depth_stencil_attachment;

  RaiiWrapper<wgpu::RenderPassEncoder> render_pass_encoder =
      command_encoder.beginRenderPass(render_pass_desc);

#pragma region setting common uniforms

  auto& camera_view = html_render_target.camera.view;
  auto& camera_settings = html_render_target.camera.settings;
  auto& camera_position = html_render_target.camera.position;

  core::CommonUniforms common{};
  common.camera.view_projection_matrix =
      camera_view.projection_matrix * camera_view.view_matrix;
  common.camera.aspect = camera_settings.aspect_ratio;
  common.camera.far = camera_settings.far;
  common.camera.near = camera_settings.near;
  common.camera.fov = camera_settings.fov;
  common.camera.position = camera_position;
  common.resolution = html_render_target.output_size;

  // TODO: make a global time component and pull common.time from there
  common.time = glfwGetTime();

  // for (auto [spotLight] : tWorld.query<Columns<SpotLight>, Tags<>>()) {
  //     assert(common.spot_light_count < 5 && "Maxiimum of 5 spotlight");
  //     common.spot_lights[common.spot_light_count] = spotLight;
  //     common.spot_light_count++;
  // }
  // for (auto [directionalLight] : tWorld.query<Columns<DirectionalLight>,
  // Tags<>>()) {
  //     assert(common.directional_light_count < 5 && "Maxiimum of 5
  //     directionalLight");
  //     common.directional_lights[common.directional_light_count] =
  //     directionalLight; common.directional_light_count++;
  // }
  // for (auto [pointLight] : tWorld.query<Columns<PointLight>, Tags<>>()) {
  //     assert(common.point_light_count < 5 && "Maxiimum of 5 pointLight");
  //     common.point_lights[common.point_light_count] = pointLight;
  //     common.point_light_count++;
  // }

  // when using the editor light attached to the editor view

  common.directional_light_count = 1;
  common.directional_lights[0].color = math::vec3(1.0f);
  // math::normalize(math::vec3(1.0f, -0.4f, 0.4f));
  common.directional_lights[0].direction =
      html_render_target.camera.orientation * math::vec3(1.0f, 0.0f, 0.0f);
  common.directional_lights[0].intensity = 1.0f;

  for (auto [ambient_light] :
       world.query<ecs::Columns<core::AmbientLight>, ecs::Tags<>>()) {
    assert(common.ambient_light_count < 5 && "Maxiimum of 5 ambientLight");
    common.ambient_lights[common.ambient_light_count] = ambient_light;
    common.ambient_light_count++;
  }

  // generalUniforms.common_uniforms->set(sizeof(common), &common);

  general.common_uniform.set(common);

#pragma endregion

#pragma region drawing each mesh
  uint32_t meshIndex = 0;
  for (auto [position, scale, orientation, mesh] :
       world.query<ecs::Columns<core::Position, core::Scale, core::Orientation,
                                core::Mesh>,
                   ecs::Tags<>>()) {
    mesh.update_gpu_data();
    // mesh.material->update(meshIndex);

    math::mat4 model_matrix =
        math::translate(math::mat4(1.0f), math::vec3(position));
    model_matrix =
        math::rotate(model_matrix, math::angle(math::quat(orientation)),
                     math::axis(math::quat(orientation)));
    model_matrix = math::scale(model_matrix, math::vec3(scale));

    auto normal_matrix = math::mat3(model_matrix);

    core::ModelUniforms model{};
    model.model_matrix = model_matrix;
    model.normal_matrix = normal_matrix;

    //   generalUniforms.model_uniforms->set(meshIndex, 0, sizeof(model),
    //   &model);
    general.model_uniform.set_active_index(meshIndex);
    general.model_uniform.set(model);

    // drawing modes modes -> surfaces (the default), edges(wireframe),
    // vertices
    std::vector<RenderObject> render_objects = {mesh.get_face_render_object()};

    for (auto& render_object : render_objects) {
      wgpu::RenderPipelineDescriptor render_pipeline_desc = wgpu::Default;
      render_pipeline_desc.vertex.bufferCount =
          render_object.vertex_buffers.size();
      render_pipeline_desc.vertex.buffers =
          render_object.vertex_buffer_layouts.data();
      render_pipeline_desc.vertex.entryPoint =
          render_object.vertex_shader_entry.c_str();
      render_pipeline_desc.vertex.module = render_object.vertex_shader;

      render_pipeline_desc.multisample.count =
          html_render_target.multisample_count;
      render_pipeline_desc.multisample.alphaToCoverageEnabled =
          html_render_target.multisample_count > 1 ? true : false;
      render_pipeline_desc.multisample.mask = ~0u;

      wgpu::FragmentState fragment_state = wgpu::Default;
      fragment_state.targets = render_target.color_targets.data();
      fragment_state.targetCount = render_target.color_targets.size();
      fragment_state.entryPoint = render_object.fragment_shader_entry.c_str();
      fragment_state.module = render_object.fragment_shader;
      render_pipeline_desc.fragment = &fragment_state;

      render_pipeline_desc.primitive.cullMode = wgpu::CullMode::None;
      render_pipeline_desc.primitive.frontFace = wgpu::FrontFace::CCW;
      render_pipeline_desc.primitive.topology = render_object.topology;

      wgpu::DepthStencilState depth_stencil_state = wgpu::Default;
      depth_stencil_state.depthCompare = wgpu::CompareFunction::Less;
      depth_stencil_state.depthWriteEnabled = true;
      depth_stencil_state.format = wgpu::TextureFormat::Depth24PlusStencil8;

      render_pipeline_desc.depthStencil = &depth_stencil_state;

#pragma region pipeline for resources
      // wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor =
      // wgpu::Default;

      // auto resourceGroupZeroEntry = resourceGroupZero.resource_group_entry();
      // auto materialResourceGroupEntry =
      // mesh.material->resource_group_entry();

      // std::vector<WGPUBindGroupLayout> bindGroupLayouts =
      // {resourceGroupZeroEntry.wgpuBindGroupLayout,
      //                                                      materialResourceGroupEntry.wgpuBindGroupLayout};
      // std::vector<WGPUBindGroupLayout> bindGroupLayouts = {
      //     resourceGroupZeroEntry.wgpuBindGroupLayout};
      // std::vector<WGPUBindGroupLayout> bindGroupLayouts = {};
      // pipelineLayoutDescriptor.bindGroupLayoutCount =
      // bindGroupLayouts.size(); pipelineLayoutDescriptor.bindGroupLayouts =
      // bindGroupLayouts.data(); RaiiWrapper<wgpu::PipelineLayout> layout =
      //     device.createPipelineLayout(pipelineLayoutDescriptor);

      Pipeline pipeline = Pipeline::create(
          "", PipelineEntries::from_sequential(general.bind_group));

      render_pipeline_desc.layout = *pipeline.pipeline_layout;
#pragma endregion

      RaiiWrapper<wgpu::RenderPipeline> render_pipeline =
          device.createRenderPipeline(render_pipeline_desc);

      // renderPassEncoder->setBindGroup(
      //     0, resourceGroupZeroEntry.wgpuBindGroup,
      //     generalUniforms.model_uniforms->stride() * meshIndex);

      for (uint32_t i = 0; i < pipeline.bind_groups.size(); i++) {
        if (pipeline.bind_groups[i]) {
          render_pass_encoder->setBindGroup(i, pipeline.bind_groups[i], 0);
        }
      }

      // renderPassEncoder->setBindGroup(1,
      // materialResourceGroupEntry.wgpuBindGroup,
      // mesh.material->uniform_offset());

      for (uint32_t i = 0; i < render_object.vertex_buffers.size(); i++) {
        render_pass_encoder->setVertexBuffer(
            i, render_object.vertex_buffers[i], 0,
            render_object.vertex_buffers[i].getSize());
      }
      render_pass_encoder->setPipeline(*render_pipeline);
      render_pass_encoder->draw(render_object.vertex_count,
                                render_object.instance_count, 0, 0);
    }
    meshIndex++;
  }
#pragma endregion

  render_pass_encoder->end();
}

RenderPass::RenderPass() : root_pass(std::make_shared<DefaultRenderPass>()) {}

void RenderPass::render(ecs::World& world,
                        HtmlCanvasRenderTarget& html_render_target) {
  auto& context = WgpuContext::get();
  auto device = context.get_device();
  auto queue = context.get_queue();

  wgpu::CommandEncoderDescriptor command_encoder_desc = wgpu::Default;
  RaiiWrapper<wgpu::CommandEncoder> command_encoder =
      device.createCommandEncoder(command_encoder_desc);

  if (root_pass) {
    (*root_pass)(*command_encoder, world, html_render_target);
  }

  wgpu::CommandBufferDescriptor command_buffer_desc = wgpu::Default;
  RaiiWrapper<wgpu::CommandBuffer> command_buffer =
      command_encoder->finish(command_buffer_desc);
  queue.submit(*command_buffer);

#if !defined(EMSCRIPTEN)
  html_render_target.swapchain->present();
#endif
}

}  // namespace renderer
