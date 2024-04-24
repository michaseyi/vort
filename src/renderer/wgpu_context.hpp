#pragma once
#include <cassert>
#include <exception>

#include "src/utils/raii.hpp"
#include "webgpu.hpp"

namespace renderer {

class WgpuContext {
 public:
  wgpu::Instance& get_instance();

  wgpu::Adapter& get_adapter();

  wgpu::Device& get_device();

  wgpu::Queue& get_queue();

  static bool init();

  static WgpuContext& get();

 private:
  WgpuContext();

 private:
  inline static WgpuContext* context_ = nullptr;

  RaiiWrapper<wgpu::Instance> instance_;
  RaiiWrapper<wgpu::Adapter> adapter_;
  RaiiWrapper<wgpu::Device> device_;
  RaiiWrapper<wgpu::Queue> queue_;
};
}  // namespace renderer