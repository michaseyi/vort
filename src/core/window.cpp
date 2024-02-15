#include "window.hpp"

#include "window_events.hpp"
Window::Window() = default;

Window::Window(GLFWwindow* tWindowHandle) : mWindowHandle(tWindowHandle) {
    if (mWindowHandle) {
        glfwGetWindowSize(mWindowHandle, &mWidth, &mHeight);

        updateUserPointer();

        glfwGetCursorPos(mWindowHandle, &prevMouseX, &prevMouseY);

        initEventHandlers();
    }
}

/**
 * @brief Needs to be called everytime the address of the window object changes.
 *
 */
void Window::updateUserPointer() {
    // TODO: Bug if the window object address changes
    glfwSetWindowUserPointer(mWindowHandle, this);
}
void Window::updateSize() {
    glfwGetWindowSize(mWindowHandle, &mWidth, &mHeight);
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

void Window::initEventHandlers() {
    glfwSetCursorPosCallback(mWindowHandle, [](GLFWwindow* windowHandle, double xPos, double yPos) {
        auto& window = *reinterpret_cast<Window*>(glfwGetWindowUserPointer(windowHandle));

        PointerMove event;
        event.x = xPos;
        event.y = yPos;
        event.movementX = xPos - window.prevMouseX;
        event.movementY = yPos - window.prevMouseY;

        event.clientWidth = window.width();
        event.clientHeight = window.height();

        window.prevMouseX = xPos;
        window.prevMouseY = yPos;
        window.emitEvent(event);
    });

    glfwSetWindowSizeCallback(mWindowHandle, [](GLFWwindow* windowHandle, int width, int height) {
        auto& window = *reinterpret_cast<Window*>(glfwGetWindowUserPointer(windowHandle));

        window.updateSize();

        WindowResize event;
        event.width = width;
        event.height = height;

        window.emitEvent(event);
    });

    glfwSetCursorEnterCallback(mWindowHandle, [](GLFWwindow* windowHandle, int entered) {
        auto& window = *reinterpret_cast<Window*>(glfwGetWindowUserPointer(windowHandle));

        if (entered == GLFW_TRUE) {
            glfwGetCursorPos(windowHandle, &window.prevMouseX, &window.prevMouseY);
        }
    });

    glfwSetMouseButtonCallback(mWindowHandle, [](GLFWwindow* windowHandle, int button, int actions, int mods) {
        auto& window = *reinterpret_cast<Window*>(glfwGetWindowUserPointer(windowHandle));

        if (!(button == GLFW_MOUSE_BUTTON_LEFT && actions == GLFW_PRESS)) {
            return;
        }

        PointerDown event;
        event.clientWidth = window.width();
        event.clientHeight = window.height();

        double x;
        double y;

        glfwGetCursorPos(windowHandle, &x, &y);

        event.x = x;
        event.y = y;

        window.emitEvent(event);
    });

    glfwSetKeyCallback(mWindowHandle, [](GLFWwindow* windowHandle, int key, int scanCode, int actions, int mods) {});
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