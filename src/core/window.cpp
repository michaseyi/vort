#include "window.hpp"

#include "window_events.hpp"

namespace core {

Window::Window() = default;

Window::Window(GLFWwindow* windwo_handle) : window_handle(windwo_handle) {
  if (window_handle) {
    glfwGetWindowSize(window_handle, &width_, &height_);

    update_user_pointer();

    glfwGetCursorPos(window_handle, &prev_mouse_x, &prev_mouse_y);

    init_event_handlers();
  }
}

/**
   * @brief Needs to be called everytime the address of the window object
   * changes.
   *
   */
void Window::update_user_pointer() {
  // TODO: Bug if the window object address changes
  glfwSetWindowUserPointer(window_handle, this);
}
void Window::update_size() {
  glfwGetWindowSize(window_handle, &width_, &height_);
}

wgpu::Surface Window::get_wgpu_surface(WGPUInstance instance) {
  return glfwGetWGPUSurface(instance, window_handle);
}

void Window::process_events() {
  glfwPollEvents();
}

void Window::wait_events() {
  glfwWaitEvents();
}

bool Window::is_open() {
  return !glfwWindowShouldClose(window_handle);
}

int32_t Window::get_width() {
  return width_;
}

int32_t Window::get_height() {
  return height_;
}

Window::operator GLFWwindow*() {
  return window_handle;
}

void Window::init_event_handlers() {
  glfwSetCursorPosCallback(
      window_handle, [](GLFWwindow* window_handle, double x_pos, double y_pox) {
        Window& window =
            *reinterpret_cast<Window*>(glfwGetWindowUserPointer(window_handle));

        PointerMove event;
        event.x = x_pos;
        event.y = y_pox;
        event.movement_x = x_pos - window.prev_mouse_x;
        event.movement_y = y_pox - window.prev_mouse_y;

        event.client_width = window.get_width();
        event.client_height = window.get_height();

        window.prev_mouse_x = x_pos;
        window.prev_mouse_y = y_pox;
        window.emit_event(event);
      });

  glfwSetWindowSizeCallback(
      window_handle, [](GLFWwindow* window_handle, int width, int height) {
        Window& window =
            *reinterpret_cast<Window*>(glfwGetWindowUserPointer(window_handle));

        window.update_size();

        WindowResize event;
        event.width = width;
        event.height = height;

        window.emit_event(event);
      });

  glfwSetCursorEnterCallback(
      window_handle, [](GLFWwindow* window_handle, int entered) {
        Window& window =
            *reinterpret_cast<Window*>(glfwGetWindowUserPointer(window_handle));

        if (entered == GLFW_TRUE) {
          glfwGetCursorPos(window_handle, &window.prev_mouse_x,
                           &window.prev_mouse_y);
        }
      });

  glfwSetMouseButtonCallback(
      window_handle,
      [](GLFWwindow* window_handle, int button, int actions, int mods) {
        Window& window =
            *reinterpret_cast<Window*>(glfwGetWindowUserPointer(window_handle));

        if (!(button == GLFW_MOUSE_BUTTON_LEFT && actions == GLFW_PRESS)) {
          return;
        }

        PointerDown event;
        event.clieht_width = window.get_width();
        event.client_height = window.get_height();

        double x;
        double y;

        glfwGetCursorPos(window_handle, &x, &y);

        event.x = x;
        event.y = y;

        window.emit_event(event);
      });

  glfwSetKeyCallback(window_handle,
                     [](GLFWwindow* window_handle, int key, int scan_code,
                        int actions, int mods) {});
}

Window::operator bool() {
  return window_handle != nullptr;
}

WindowBuilder::WindowBuilder() {
  glfwInit();
  glfwDefaultWindowHints();
}

WindowBuilder& WindowBuilder::client_api(ClientApi client_api) {
  glfwWindowHint(GLFW_CLIENT_API, static_cast<int32_t>(client_api));
  return *this;
}

WindowBuilder& WindowBuilder::resizable(bool value) {
  glfwWindowHint(GLFW_RESIZABLE, static_cast<int>(value));
  return *this;
}

WindowBuilder& WindowBuilder::title(std::string title) {
  title_ = title;
  return *this;
}

WindowBuilder& WindowBuilder::size(int32_t width, int32_t height) {
  width_ = width;
  height_ = height;
  return *this;
}

Window WindowBuilder::build() {
  return glfwCreateWindow(width_, height_, title_.c_str(), nullptr, nullptr);
}
}  // namespace core