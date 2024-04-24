#pragma once
#include <GLFW/glfw3.h>
#include <glfw3webgpu/glfw3webgpu.h>

#include <cstdint>
#include <iostream>
#include <string>

#include "event_emitter.hpp"
#include "src/renderer/wgpu_context.hpp"
#include "src/utils/raii.hpp"

template <>
struct RaiiDeleter<GLFWwindow*> {
  static void deleter(GLFWwindow* raw) { glfwDestroyWindow(raw); }
};
namespace core {

class Window : public EventEmitter {
 public:
  Window();

  Window(GLFWwindow* window_handle);

  wgpu::Surface get_wgpu_surface(WGPUInstance instance);

  void process_events();

  void update_user_pointer();

  void wait_events();

  bool is_open();

  int32_t get_width();

  int32_t get_height();

  void update_size();

  operator GLFWwindow*();

  operator bool();

  double prev_mouse_x = 0;
  double prev_mouse_y = 0;

 private:
  void init_event_handlers();

  int32_t width_ = 0;
  int32_t height_ = 0;
  RaiiWrapper<GLFWwindow*> window_handle;
};

enum class ClientApi {
  None = 0,
};

class WindowBuilder {
 public:
  WindowBuilder();

  WindowBuilder& client_api(ClientApi client_api);

  WindowBuilder& resizable(bool value);

  WindowBuilder& title(std::string title);

  WindowBuilder& size(int32_t width, int32_t height);

  Window build();

 private:
  int32_t width_ = 0;
  int32_t height_ = 0;
  std::string title_;
};
}  // namespace core