#pragma once
#include "src/utils/raii.hpp"
#include "wgpu_context.hpp"

template <>
struct RAIIDeleter<wgpu::Buffer> {
    static void deleter(wgpu::Buffer &tBuffer) {
        tBuffer.destroy();
        tBuffer.release();
    }
};

template <>
struct RAIIDeleter<wgpu::Texture> {
    static void deleter(wgpu::Texture &tTexture) {
        tTexture.destroy();
        tTexture.release();
    }
};