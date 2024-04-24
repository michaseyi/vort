#pragma once
#include "src/utils/raii.hpp"
#include "wgpu_context.hpp"

template <>
struct RaiiDeleter<wgpu::Buffer> {
  static void deleter(wgpu::Buffer& buffer) {
    buffer.destroy();
    buffer.release();
  }
};

template <>
struct RaiiDeleter<wgpu::Texture> {
  static void deleter(wgpu::Texture& texture) {
    texture.destroy();
    texture.release();
  }
};

template <>
struct RaiiDeleter<wgpu::SwapChain> {
  static void deleter(wgpu::SwapChain& swapchain) { swapchain.release(); }
};