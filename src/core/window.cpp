#include "window.hpp"

Window::Window() = default;

Window::Window(GLFWwindow* tWindowHandle) : mWindowHandle(tWindowHandle) {
    if (mWindowHandle) {
        glfwGetWindowSize(mWindowHandle, &mWidth, &mHeight);

        glfwSetWindowUserPointer(mWindowHandle, this);
    }
}

wgpu::Surface Window::getWGPUSurface(WGPUInstance tInstance) {
    return glfwGetWGPUSurface(tInstance, mWindowHandle);
}

void Window::processEvents() {
    glfwPollEvents();
}

void Window::waitEvents() {
    glfwWaitEvents();
}

bool Window::isOpen() {
    return !glfwWindowShouldClose(mWindowHandle);
}

int32_t Window::width() {
    return mWidth;
}

int32_t Window::height() {
    return mHeight;
}

Window::operator GLFWwindow*() {
    return mWindowHandle;
}

Window::operator bool() {
    return mWindowHandle != nullptr;
}

WindowBuilder::WindowBuilder() {
    glfwInit();
    glfwDefaultWindowHints();
}

WindowBuilder& WindowBuilder::clientAPI(ClientAPI tClientAPI) {
    glfwWindowHint(GLFW_CLIENT_API, static_cast<int32_t>(tClientAPI));
    return *this;
}

WindowBuilder& WindowBuilder::resizable(bool tValue) {
    glfwWindowHint(GLFW_RESIZABLE, static_cast<int>(tValue));
    return *this;
}

WindowBuilder& WindowBuilder::title(std::string tTitle) {
    mTitle = tTitle;
    return *this;
}

WindowBuilder& WindowBuilder::size(int32_t tWidth, int32_t tHeight) {
    mWidth = tWidth;
    mHeight = tHeight;
    return *this;
}

Window WindowBuilder::build() {
    return glfwCreateWindow(mWidth, mHeight, mTitle.c_str(), nullptr, nullptr);
}