#include "wgpu_engine.hpp"

void WgpuEngine::init() {
  assert(singleton_instance_ == nullptr);

  wgpu::InstanceDescriptor instance_desc = {};
  instance_ = wgpu::createInstance(instance_desc);

  wgpu::RequestAdapterOptions adapter_options = {};
  adapter_ = instance_.requestAdapter(adapter_options);

  wgpu::DeviceDescriptor device_desc = {};
  device_ = adapter_.requestDevice(device_desc);

#if !defined(EMSCRIPTEN)
  static auto deviceLostCallbackHandle = device_.setDeviceLostCallback(
      [](wgpu::DeviceLostReason reason, const char* message) {
        std::cout << "[Device lost]: reason " << reason;
        if (message) {
          std::cout << " (" << message << ")";
        }
        std::cout << std::endl;
      });
#endif

  static auto uncapturedErrorCallbackHandle =
      device_.setUncapturedErrorCallback(
          [](wgpu::ErrorType error_type, const char* message) {
            std::cout << "[Uncaptured device error]: type " << error_type;
            if (message) {
              std::cout << " (" << message << ")";
            }
            std::cout << std::endl;
          });

  initialized_ = true;
}

void WgpuEngine::clean_up() {
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