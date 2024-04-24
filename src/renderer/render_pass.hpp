#pragma once
#include <iostream>
#include <memory>
#include <vector>

#include "bind_group.hpp"
#include "dynamic_uniform_buffer.hpp"
#include "html_canvas_render_target.hpp"
#include "sampler.hpp"
#include "src/ecs/ecs.hpp"
#include "src/utils/raii.hpp"
#include "texture.hpp"
#include "uniform_buffer.hpp"

namespace renderer {
struct General {
  renderer::BindGroup bind_group;
  renderer::UniformBuffer<core::CommonUniforms> common_uniform;
  std::vector<renderer::Texture> textures;
  renderer::Sampler sampler;
  renderer::DynamicUniformBuffer<core::ModelUniforms> model_uniform;
};
struct GeneralUniforms {
  std::shared_ptr<Uniform> common_uniforms;
  std::shared_ptr<Uniform> model_uniforms;
};

struct RenderTarget {
  std::vector<wgpu::ColorTargetState> color_targets;
  std::vector<wgpu::RenderPassColorAttachment> color_attachments;
  std::vector<RaiiWrapper<wgpu::TextureView>> texture_views;
  std::vector<wgpu::BlendState> color_target_blend_state;
};
struct Pass {
  virtual void operator()(wgpu::CommandEncoder& command_encoder,
                          ecs::World& world,
                          HtmlCanvasRenderTarget& html_render_target) = 0;

  virtual RenderTarget render_targets(
      HtmlCanvasRenderTarget& html_render_target) = 0;

  void add_subpasses(std::shared_ptr<Pass> pass);

  std::vector<std::shared_ptr<Pass>> subpasses;

  virtual ~Pass() = default;
};

struct DefaultRenderPass : public Pass {
  DefaultRenderPass() = default;

  RenderTarget render_targets(
      HtmlCanvasRenderTarget& html_render_target) override;

  void operator()(wgpu::CommandEncoder& command_encoder, ecs::World& world,
                  HtmlCanvasRenderTarget& html_render_target) override;
};

struct RenderPass {
  RenderPass();

  void render(ecs::World& world, HtmlCanvasRenderTarget& html_render_target);

 private:
  std::shared_ptr<Pass> root_pass;
};
}  // namespace renderer