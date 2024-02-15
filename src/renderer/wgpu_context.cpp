

#define WEBGPU_CPP_IMPLEMENTATION
#include "wgpu_context.hpp"

#include "raii_deleters.hpp"
#include "src/core/window.hpp"
#include "src/core/window_events.hpp"

using namespace wgpu;

Instance& WGPUContext::getInstance() {
    return *mInstance;
}

Adapter& WGPUContext::getAdapter() {
    return *mAdapter;
}

Device WGPUContext::getDevice() {
    return *mDevice;
}

SwapChain WGPUContext::getSwapChain() {
    return *mSwapChain;
}

Surface WGPUContext::getSurface() {
    return *mSurface;
}

Queue WGPUContext::getQueue() {
    return *mQueue;
}

RAIIWrapper<wgpu::TextureView> WGPUContext::getDepthStencilTextureView() {
    wgpu::TextureViewDescriptor textureViewDesc = wgpu::Default;
    textureViewDesc.arrayLayerCount = 1;
    textureViewDesc.baseArrayLayer = 0;
    textureViewDesc.mipLevelCount = 1;
    textureViewDesc.baseMipLevel = 0;
    textureViewDesc.aspect = wgpu::TextureAspect::All;
    textureViewDesc.dimension = wgpu::TextureViewDimension::_2D;
    textureViewDesc.format = mDepthStencilTextureFormat;

    return mDepthStencilTexture->createView(textureViewDesc);
}

RAIIWrapper<wgpu::TextureView> WGPUContext::getMultiSampleTextureView() {
    wgpu::TextureViewDescriptor textureViewDesc = wgpu::Default;
    textureViewDesc.arrayLayerCount = 1;
    textureViewDesc.baseArrayLayer = 0;
    textureViewDesc.mipLevelCount = 1;
    textureViewDesc.baseMipLevel = 0;
    textureViewDesc.aspect = wgpu::TextureAspect::All;
    textureViewDesc.dimension = wgpu::TextureViewDimension::_2D;
    textureViewDesc.format = mOutputTextureFormat;
    return mMultiSamplingTexture->createView(textureViewDesc);
}

wgpu::TextureFormat WGPUContext::getOutputTextureFormat() {
    return mOutputTextureFormat;
}

wgpu::TextureFormat WGPUContext::getDepthStencilTextureFormat() {
    return mDepthStencilTextureFormat;
}

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
    assert(mContext && "WGPUContext not created. Must call WGPUContext::init before WGPUContext::getContext");
    return *mContext;
}

SwapChain WGPUContext::createSwapChain(int32_t tWidth, int32_t tHeight) {
    SwapChainDescriptor swapChainDesc = Default;
    swapChainDesc.presentMode = PresentMode::Fifo;
    swapChainDesc.height = tHeight;
    swapChainDesc.width = tWidth;
    swapChainDesc.format = mOutputTextureFormat;
    swapChainDesc.usage = TextureUsage::RenderAttachment;

    return mDevice->createSwapChain(*mSurface, swapChainDesc);
}

wgpu::Texture WGPUContext::createMultiSamplingTexture(int32_t tWidth, int32_t tHeight) {
    wgpu::TextureDescriptor multisamplingTextureDesc = wgpu::Default;
    multisamplingTextureDesc.dimension = wgpu::TextureDimension::_2D;
    multisamplingTextureDesc.format = mOutputTextureFormat;
    multisamplingTextureDesc.mipLevelCount = 1;
    multisamplingTextureDesc.sampleCount = mMultiSamplingCount;
    multisamplingTextureDesc.size = {static_cast<uint32_t>(tWidth), static_cast<uint32_t>(tHeight), 1};
    multisamplingTextureDesc.viewFormatCount = 1;
    multisamplingTextureDesc.viewFormats = reinterpret_cast<WGPUTextureFormat*>(&mOutputTextureFormat);
    multisamplingTextureDesc.usage = wgpu::TextureUsage::RenderAttachment;
    return mDevice->createTexture(multisamplingTextureDesc);
}
wgpu::Texture WGPUContext::createDepthStencilTexture(int32_t tWidth, int32_t tHeight) {
    wgpu::TextureDescriptor depthStencilTextureDesc = wgpu::Default;
    depthStencilTextureDesc.dimension = wgpu::TextureDimension::_2D;
    depthStencilTextureDesc.format = mDepthStencilTextureFormat;
    depthStencilTextureDesc.mipLevelCount = 1;
    depthStencilTextureDesc.sampleCount = mMultiSamplingCount;
    depthStencilTextureDesc.size = {static_cast<uint32_t>(tWidth), static_cast<uint32_t>(tHeight), 1};
    depthStencilTextureDesc.viewFormatCount = 1;
    depthStencilTextureDesc.viewFormats = reinterpret_cast<WGPUTextureFormat*>(&mDepthStencilTextureFormat);
    depthStencilTextureDesc.usage = wgpu::TextureUsage::RenderAttachment;
    return mDevice->createTexture(depthStencilTextureDesc);
}

void WGPUContext::onWindowResize(int32_t tWidth, int32_t tHeight) {
    mSwapChain = createSwapChain(tWidth, tHeight);
    mMultiSamplingTexture = createMultiSamplingTexture(tWidth, tHeight);
    mDepthStencilTexture = createDepthStencilTexture(tWidth, tHeight);
}

WGPUContext::WGPUContext(Window* tWindow) {
    assert(tWindow);

    InstanceDescriptor desc = Default;
    mInstance = createInstance(desc);

    mSurface = tWindow->getWGPUSurface(*mInstance);
    RequestAdapterOptions adapterOptions = Default;
    adapterOptions.compatibleSurface = *mSurface;

    mAdapter = mInstance->requestAdapter(adapterOptions);

    DeviceDescriptor deviceDesc = Default;
    mDevice = mAdapter->requestDevice(deviceDesc);

    mQueue = mDevice->getQueue();

    mOutputTextureFormat = wgpu::TextureFormat::BGRA8Unorm;
    mDepthStencilTextureFormat = wgpu::TextureFormat::Depth24PlusStencil8;
    mMultiSamplingCount = 4;

    mSwapChain = createSwapChain(tWindow->width(), tWindow->height());

    mMultiSamplingTexture = createMultiSamplingTexture(tWindow->width(), tWindow->height());

    mDepthStencilTexture = createDepthStencilTexture(tWindow->width(), tWindow->height());

    tWindow->registerListener<WindowResize>([](const WindowResize* event) {
        auto& context = WGPUContext::getContext();
        context.onWindowResize(event->width, event->height);
    });

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