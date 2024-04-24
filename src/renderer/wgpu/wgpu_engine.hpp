#include "wgpu_types.hpp"

class WgpuEngine {
 public:
  void clean_up();
  void draw();
  void init();
  bool initialized();
  
  static WgpuEngine& get();

 private:
  wgpu::Instance instance_ = nullptr;
  wgpu::Adapter adapter_ = nullptr;
  wgpu::Device device_ = nullptr;
  inline static WgpuEngine* singleton_instance_ = nullptr;
  bool initialized_ = false;
};