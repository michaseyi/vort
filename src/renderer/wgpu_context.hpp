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

    wgpu::Surface getSurface();

    wgpu::Queue getQueue();

    static bool init(Window* tWindow = nullptr);

    static WGPUContext& getContext();

    wgpu::SwapChain createSwapChain(int32_t tWidth, int32_t tHeight);

private:
    WGPUContext(Window* tWindow);

    inline static WGPUContext* mContext = nullptr;

    RAIIWrapper<wgpu::Instance> mInstance;
    RAIIWrapper<wgpu::Adapter> mAdapter;
    RAIIWrapper<wgpu::Device> mDevice;
    RAIIWrapper<wgpu::SwapChain> mSwapChain;
    RAIIWrapper<wgpu::Queue> mQueue;
    RAIIWrapper<wgpu::Surface> mSurface;
};