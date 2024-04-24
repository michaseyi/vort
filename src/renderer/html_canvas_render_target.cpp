#include "raii_deleters.hpp"
#include "src/bindings/ecs_binding.hpp"
#include "src/core/camera_manager.hpp"
#include "src/utils/raii.hpp"
#include "wgpu_context.hpp"

namespace renderer {

wgpu::Surface HtmlCanvasRenderTarget::create_surface() {
  wgpu::SurfaceDescriptorFromCanvasHTMLSelector canvas = wgpu::Default;
  canvas.chain.sType = wgpu::SType::SurfaceDescriptorFromCanvasHTMLSelector;
  canvas.selector = canvas_selector.c_str();
  wgpu::SurfaceDescriptor surface_desc = wgpu::Default;
  surface_desc.nextInChain = (const wgpu::ChainedStruct*)&canvas;
  return WgpuContext::get().get_instance().createSurface(surface_desc);
}

wgpu::SwapChain HtmlCanvasRenderTarget::create_swapchain() {
  wgpu::SwapChainDescriptor swapchain_desc = wgpu::Default;
  swapchain_desc.format = output_texture_format;
  swapchain_desc.usage = wgpu::TextureUsage::RenderAttachment;
  swapchain_desc.width = output_size.x;
  swapchain_desc.height = output_size.y;
  swapchain_desc.presentMode = wgpu::PresentMode::Fifo;

  return WgpuContext::get().get_device().createSwapChain(surface,
                                                         swapchain_desc);
}

wgpu::Texture HtmlCanvasRenderTarget::create_multisample_texture() {
  wgpu::TextureDescriptor multisample_texture_desc = wgpu::Default;
  multisample_texture_desc.dimension = wgpu::TextureDimension::_2D;
  multisample_texture_desc.format = output_texture_format;
  multisample_texture_desc.mipLevelCount = 1;
  multisample_texture_desc.sampleCount = multisample_count;
  multisample_texture_desc.size = {static_cast<uint32_t>(output_size.x),
                                   static_cast<uint32_t>(output_size.y), 1};
  multisample_texture_desc.viewFormatCount = 1;
  multisample_texture_desc.viewFormats =
      reinterpret_cast<WGPUTextureFormat*>(&output_texture_format);
  multisample_texture_desc.usage = wgpu::TextureUsage::RenderAttachment;
  return WgpuContext::get().get_device().createTexture(
      multisample_texture_desc);
}
wgpu::Texture HtmlCanvasRenderTarget::create_depth_stencil_texture() {
  wgpu::TextureDescriptor depth_stencil_texture_desc = wgpu::Default;
  depth_stencil_texture_desc.dimension = wgpu::TextureDimension::_2D;
  depth_stencil_texture_desc.format = depth_stencil_texture_format;
  depth_stencil_texture_desc.mipLevelCount = 1;
  depth_stencil_texture_desc.sampleCount = multisample_count;
  depth_stencil_texture_desc.size = {static_cast<uint32_t>(output_size.x),
                                     static_cast<uint32_t>(output_size.y), 1};
  depth_stencil_texture_desc.viewFormatCount = 1;
  depth_stencil_texture_desc.viewFormats =
      reinterpret_cast<WGPUTextureFormat*>(&depth_stencil_texture_format);
  depth_stencil_texture_desc.usage = wgpu::TextureUsage::RenderAttachment;
  return WgpuContext::get().get_device().createTexture(
      depth_stencil_texture_desc);
}

HtmlCanvasRenderTarget::HtmlCanvasRenderTarget(std::string canvas_selector)
    : canvas_selector(canvas_selector) {
  output_size = bindings::EcsBinding::get_canvas_size(canvas_selector.c_str());
  multisample_count = 4;
  output_texture_format = wgpu::TextureFormat::BGRA8Unorm;
  depth_stencil_texture_format = wgpu::TextureFormat::Depth24PlusStencil8;
  render_count = -1;
  camera = core::CameraManager(
      math::vec3(15.0f, 0, 0),
      core::CameraSettings(
          40.0f,
          static_cast<float>(output_size.x) / static_cast<float>(output_size.y),
          0.01f, 1000.0f));
  camera.rotate(math::radians(-45.0f * 100), math::radians(45.0f * 100));

  surface = create_surface();
  swapchain = create_swapchain();
  depth_stencil_texture = create_depth_stencil_texture();

  if (multisample_count > 1) {
    multisample_texture = create_multisample_texture();
  }
}

void HtmlCanvasRenderTarget::update_camera() {
  camera.settings.aspect_ratio =
      static_cast<float>(output_size.x) / static_cast<float>(output_size.y);
  camera.update_view();
}

void HtmlCanvasRenderTarget::update() {
  bindings::EcsBinding::update_canvas_size(canvas_selector.c_str());

  swapchain = create_swapchain();
  depth_stencil_texture = create_depth_stencil_texture();

  if (multisample_count > 1) {
    multisample_texture = create_multisample_texture();
  }
}

RaiiWrapper<wgpu::TextureView>
HtmlCanvasRenderTarget::get_current_texture_view() {
  return swapchain->getCurrentTextureView();
}

RaiiWrapper<wgpu::TextureView>
HtmlCanvasRenderTarget::get_depth_stencil_texture_view() {
  wgpu::TextureViewDescriptor texture_view_desc = wgpu::Default;
  texture_view_desc.arrayLayerCount = 1;
  texture_view_desc.baseArrayLayer = 0;
  texture_view_desc.mipLevelCount = 1;
  texture_view_desc.baseMipLevel = 0;
  texture_view_desc.aspect = wgpu::TextureAspect::All;
  texture_view_desc.dimension = wgpu::TextureViewDimension::_2D;
  texture_view_desc.format = depth_stencil_texture_format;

  return depth_stencil_texture->createView(texture_view_desc);
}

RaiiWrapper<wgpu::TextureView>
HtmlCanvasRenderTarget::get_multisample_texture_view() {
  wgpu::TextureViewDescriptor texture_view_desc = wgpu::Default;
  texture_view_desc.arrayLayerCount = 1;
  texture_view_desc.baseArrayLayer = 0;
  texture_view_desc.mipLevelCount = 1;
  texture_view_desc.baseMipLevel = 0;
  texture_view_desc.aspect = wgpu::TextureAspect::All;
  texture_view_desc.dimension = wgpu::TextureViewDimension::_2D;
  texture_view_desc.format = output_texture_format;
  return multisample_texture->createView(texture_view_desc);
}

}  // namespace renderer