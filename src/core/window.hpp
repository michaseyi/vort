#pragma once
#include <GLFW/glfw3.h>
#include <glfw3webgpu/glfw3webgpu.h>

#include <cstdint>
#include <iostream>
#include <string>

#include "src/renderer/wgpu_context.hpp"
#include "src/utils/raii.hpp"

template <>
struct RAIIDeleter<GLFWwindow*> {
    static void deleter(GLFWwindow* tRaw) {
        glfwDestroyWindow(tRaw);
    }
};

class Window {
public:
    Window();

    Window(GLFWwindow* tWindowHandle);

    wgpu::Surface getWGPUSurface(WGPUInstance tInstance);

    void processEvents();

    void waitEvents();

    bool isOpen();

    int32_t width();

    int32_t height();

    operator GLFWwindow*();

    operator bool();

private:
    int32_t mWidth = 0;
    int32_t mHeight = 0;
    RAIIWrapper<GLFWwindow*> mWindowHandle;
};

enum class ClientAPI {
    None = 0,
};

class WindowBuilder {
public:
    WindowBuilder();

    WindowBuilder& clientAPI(ClientAPI tClientAPI);

    WindowBuilder& resizable(bool tValue);

    WindowBuilder& title(std::string tTitle);

    WindowBuilder& size(int32_t tWidth, int32_t tHeight);

    Window build();

private:
    int32_t mWidth = 0;
    int32_t mHeight = 0;
    std::string mTitle;
};