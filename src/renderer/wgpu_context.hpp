#pragma once
#include <cassert>
#include <exception>

#include "src/utils/raii.hpp"
#include "webgpu.hpp"

class Window;

class WGPUContext {
public:
    wgpu::Instance& getInstance();

    wgpu::Adapter& getAdapter();

    wgpu::Device getDevice();

    wgpu::SwapChain getSwapChain();

    RAIIWrapper<wgpu::TextureView> getDepthStencilTextureView();

    RAIIWrapper<wgpu::TextureView> getMultiSampleTextureView();

    wgpu::Surface getSurface();

    wgpu::Queue getQueue();

    static bool init(Window* tWindow = nullptr);

    static WGPUContext& getContext();

    void onWindowResize(int32_t tWidth, int32_t tHeight);

    wgpu::TextureFormat getOutputTextureFormat();

    wgpu::TextureFormat getDepthStencilTextureFormat();

private:
    wgpu::SwapChain createSwapChain(int32_t tWidth, int32_t tHeight);
    wgpu::Texture createMultiSamplingTexture(int32_t tWidth, int32_t tHeight);
    wgpu::Texture createDepthStencilTexture(int32_t tWidth, int32_t tHeight);
    WGPUContext(Window* tWindow);

private:
    inline static WGPUContext* mContext = nullptr;

    RAIIWrapper<wgpu::Instance> mInstance;
    RAIIWrapper<wgpu::Adapter> mAdapter;
    RAIIWrapper<wgpu::Device> mDevice;
    RAIIWrapper<wgpu::Queue> mQueue;
    RAIIWrapper<wgpu::Surface> mSurface;

    // to be recreated when window changes size changes
    RAIIWrapper<wgpu::SwapChain> mSwapChain;
    RAIIWrapper<wgpu::Texture> mMultiSamplingTexture;
    RAIIWrapper<wgpu::Texture> mDepthStencilTexture;

    wgpu::TextureFormat mOutputTextureFormat = wgpu::TextureFormat::Undefined;
    wgpu::TextureFormat mDepthStencilTextureFormat = wgpu::TextureFormat::Undefined;

    uint32_t mMultiSamplingCount = 4;
};