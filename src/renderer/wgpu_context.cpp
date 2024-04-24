

#define WEBGPU_CPP_IMPLEMENTATION
#include "wgpu_context.hpp"

#include "raii_deleters.hpp"

namespace renderer {

wgpu::Instance& WgpuContext::get_instance() {
  return *instance_;
}

wgpu::Adapter& WgpuContext::get_adapter() {
  return *adapter_;
}

wgpu::Device& WgpuContext::get_device() {
  return *device_;
}

wgpu::Queue& WgpuContext::get_queue() {
  return *queue_;
}

bool WgpuContext::init() {
  if (!context_) {
    try {
      context_ = new WgpuContext();
    } catch (const std::exception&) {
      return false;
    }
  }
  return true;
}
WgpuContext& WgpuContext::get() {
  assert(context_ &&
         "WgpuContext not created. Must call WgpuContext::init before "
         "WgpuContext::get");
  return *context_;
}

WgpuContext::WgpuContext() {
  wgpu::InstanceDescriptor desc = wgpu::Default;
  instance_ = createInstance(desc);

  wgpu::RequestAdapterOptions adapterOptions = wgpu::Default;

  adapter_ = instance_->requestAdapter(adapterOptions);

  wgpu::DeviceDescriptor deviceDesc = wgpu::Default;
  device_ = adapter_->requestDevice(deviceDesc);

  queue_ = device_->getQueue();

#if !defined(EMSCRIPTEN)
  static auto device_lost_callback_handle = device_->setDeviceLostCallback(
      [](wgpu::DeviceLostReason reason, const char* message) {
        std::cout << "Device lost: reason " << reason;
        if (message) {
          std::cout << " (" << message << ")";
        }
        std::cout << std::endl;
      });
#endif

  static auto uncaptured_error_callback_handle =
      device_->setUncapturedErrorCallback(
          [](wgpu::ErrorType error_type, const char* message) {
            std::cout << "Uncaptured device error: type " << error_type;
            if (message) {
              std::cout << " (" << message << ")";
            }
            std::cout << std::endl;
          });
}
}  // namespace renderer