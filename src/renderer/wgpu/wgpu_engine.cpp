#include "wgpu_engine.hpp"

void WgpuEngine::init() {
  assert(singleton_instance_ == nullptr);

  wgpu::InstanceDescriptor instance_descriptor{};
  instance_ = wgpu::createInstance(instance_descriptor);

  wgpu::RequestAdapterOptions adapter_options{};
  adapter_ = instance_.requestAdapter(adapter_options);

  wgpu::DeviceDescriptor device_descriptor{};
  device_ = adapter_.requestDevice(device_descriptor);

  queue_ = device_.getQueue();

#if !defined(EMSCRIPTEN)
  device_lost_callback_ = device_.setDeviceLostCallback(
      [](wgpu::DeviceLostReason reason, const char* message) {
        std::cout << "[Device lost]: reason " << reason;
        if (message) {
          std::cout << " (" << message << ")";
        }
        std::cout << std::endl;
      });
#endif

  uncaptured_error_callback_ = device_.setUncapturedErrorCallback(
      [](wgpu::ErrorType error_type, const char* message) {
        std::cout << "[Uncaptured device error]: type " << error_type;
        if (message) {
          std::cout << " (" << message << ")";
        }
        std::cout << std::endl;
      });

  initialized_ = true;
}

void WgpuEngine::cleanup() {
  if (initialized_) {
    device_.release();
    adapter_.release();
    instance_.release();

    initialized_ = false;
    singleton_instance_ = nullptr;
  }
}
WgpuEngine& WgpuEngine::get() {
  return *singleton_instance_;
}

bool WgpuEngine::initialized() {
  return initialized_;
}

void WgpuEngine::draw() {
  auto encoder = device_.createCommandEncoder(wgpu::CommandEncoderDescriptor{});
  wgpu::RenderPassDescriptor render_pass_descriptor{};

  wgpu::FragmentState a;
  wgpu::ColorTargetState b;
  wgpu::BlendState c;
  encoder.beginRenderPass(render_pass_descriptor);

  queue_.submit(encoder.finish());


}