

#define WEBGPU_CPP_IMPLEMENTATION
#include "wgpu_context.hpp"

#include "src/core/window.hpp"

using namespace wgpu;

Instance& WGPUContext::getInstance() { return *mInstance; }

Adapter& WGPUContext::getAdapter() { return *mAdapter; }

Device WGPUContext::getDevice() { return *mDevice; }

SwapChain WGPUContext::getSwapChain() { return *mSwapChain; }

Surface WGPUContext::getSurface() { return *mSurface; }

Queue WGPUContext::getQueue() { return *mQueue; }

bool WGPUContext::init(Window* tWindow) {
    if (!mContext) {
        try {
            mContext = new WGPUContext(tWindow);
        } catch (const std::exception&) {
            return false;
        }
    }
    return true;
}
WGPUContext& WGPUContext::getContext() {
    assert(mContext && "WGPUContext not created. Must call WGPUContext::init before WGPUContext::get");
    return *mContext;
}

SwapChain WGPUContext::createSwapChain(int32_t tWidth, int32_t tHeight) {
    SwapChainDescriptor swapChainDesc = Default;
    swapChainDesc.presentMode = PresentMode::Fifo;
    swapChainDesc.height = tHeight;
    swapChainDesc.width = tWidth;
    swapChainDesc.format = TextureFormat::BGRA8Unorm;
    swapChainDesc.usage = TextureUsage::RenderAttachment;

    return mDevice->createSwapChain(*mSurface, swapChainDesc);
}

WGPUContext::WGPUContext(Window* tWindow) {
    InstanceDescriptor desc = Default;
    mInstance = createInstance(desc);

    if (tWindow) {
        mSurface = tWindow->getWGPUSurface(*mInstance);
    }
    RequestAdapterOptions adapterOptions = Default;
    adapterOptions.compatibleSurface = *mSurface;

    mAdapter = mInstance->requestAdapter(adapterOptions);

    DeviceDescriptor deviceDesc = Default;
    mDevice = mAdapter->requestDevice(deviceDesc);

    mQueue = mDevice->getQueue();

    if (tWindow) {
        mSwapChain = createSwapChain(tWindow->width(), tWindow->height());
    }

#if !defined(EMSCRIPTEN)
    static auto deviceLostCallbackHandle = mDevice->setDeviceLostCallback([](DeviceLostReason reason, const char* message) {
        std::cout << "Device lost: reason " << reason;
        if (message) {
            std::cout << " (" << message << ")";
        }
        std::cout << std::endl;
    });
#endif

    static auto uncapturedErrorCallbackHandle = mDevice->setUncapturedErrorCallback([](ErrorType errorType, const char* message) {
        std::cout << "Uncaptured device error: type " << errorType;
        if (message) {
            std::cout << " (" << message << ")";
        }
        std::cout << std::endl;
    });
}