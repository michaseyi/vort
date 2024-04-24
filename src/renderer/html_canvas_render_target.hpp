#pragma once
#include "raii_deleters.hpp"
#include "src/core/camera_manager.hpp"
#include "src/utils/raii.hpp"
#include "wgpu_context.hpp"

namespace renderer {

struct OutputRenderTarget {
  math::ivec2 output_size;
  bool update_on_next_render = false;
  int32_t render_count;
  int32_t multisample_count;
  RaiiWrapper<wgpu::Texture> multisample_texture;
  RaiiWrapper<wgpu::Texture> depth_stencil_texture;
  RaiiWrapper<wgpu::Surface> surface;
  RaiiWrapper<wgpu::SwapChain> swapchain;
  wgpu::TextureFormat output_texture_format = wgpu::TextureFormat::Undefined;
  wgpu::TextureFormat depth_stencil_texture_format =
      wgpu::TextureFormat::Undefined;
};

struct NativeRenderTarget : public OutputRenderTarget {};

struct HtmlCanvasRenderTarget : public OutputRenderTarget {
  std::string canvas_selector;
  core::CameraManager camera;

  wgpu::Surface create_surface();

  wgpu::SwapChain create_swapchain();

  wgpu::Texture create_multisample_texture();

  wgpu::Texture create_depth_stencil_texture();

  HtmlCanvasRenderTarget(std::string canvas_selector);

  void update_camera();

  void update();

  RaiiWrapper<wgpu::TextureView> get_current_texture_view();

  RaiiWrapper<wgpu::TextureView> get_depth_stencil_texture_view();

  RaiiWrapper<wgpu::TextureView> get_multisample_texture_view();
};
}  // namespace renderer