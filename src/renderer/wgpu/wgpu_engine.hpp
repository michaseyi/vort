#include <expected>
#include "wgpu_types.hpp"

class WgpuEngine {
 public:
  void cleanup();
  void draw();
  void init();
  bool initialized();

  static WgpuEngine& get();

  
 private:
  wgpu::Instance instance_ = nullptr;
  wgpu::Adapter adapter_ = nullptr;
  wgpu::Device device_ = nullptr;
  wgpu::Queue queue_ = nullptr;
  wgpu::Extent3D draw_extent_ = wgpu::Extent3D(1, 2, 3);
  wgpu::Texture draw_image_ = nullptr;

  bool initialized_ = false;

  std::unique_ptr<wgpu::ErrorCallback> uncaptured_error_callback_;

#if !defined(EMSCRIPTEN)
  std::unique_ptr<wgpu::DeviceLostCallback> device_lost_callback_;
#endif

  inline static WgpuEngine* singleton_instance_ = nullptr;
};