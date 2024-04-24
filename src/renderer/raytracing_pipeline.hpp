#include "render_pass.hpp"
#include "resource.hpp"

namespace renderer {
struct RaytracingPass : public Pass {
  RaytracingPass() = default;

  RenderTarget render_targets(
      HtmlCanvasRenderTarget& canvas_render_target) override {
    return RenderTarget();
  }

  void operator()(wgpu::CommandEncoder& command_encoder, ecs::World& world,
                  HtmlCanvasRenderTarget& canvas_render_target) override {
    auto query = world.query<ecs::Columns<core::Position, core::Scale,
                                          core::Orientation, core::Mesh>,
                             ecs::Tags<>>();

    for (auto i = query.begin(); i != query.end(); ++i) {
      auto [position, scale, orientation, mesh] = *i;
    }
    WgpuContext& context = WgpuContext::get();
    wgpu::Device device = context.get_device();

    wgpu::ComputePassDescriptor compute_pass_desc = wgpu::Default;

    compute_pass_desc.label = "Raytracing Pass";

    RaiiWrapper<wgpu::ComputePassEncoder> compute_pass =
        command_encoder.beginComputePass(compute_pass_desc);

    wgpu::ComputePipelineDescriptor compute_pipeline_desc = wgpu::Default;

    compute_pipeline_desc.compute.entryPoint = "main";
    //   computePipelineDesc.compute.module = ;

    RaiiWrapper<wgpu::ComputePipeline> compute_pipeline =
        device.createComputePipeline(compute_pipeline_desc);

    compute_pass->setPipeline(*compute_pipeline);
    compute_pass->dispatchWorkgroups(1, 1, 1);
    compute_pass->end();
  }
};
}  // namespace renderer