
#pragma once

#include <cassert>
#include <iostream>

#include "resource.hpp"
#include "src/utils/raii.hpp"
#include "wgpu_context.hpp"

class Uniform : public Resource {
public:
    uint32_t ceilToNextMultiple(uint32_t tValue, uint32_t tStep) {
        uint32_t divideAndCeil = tValue / tStep + (tValue % tStep == 0 ? 0 : 1);

        return tStep * divideAndCeil;
    }

    uint32_t computeStride(uint32_t tOriginalSize) {
        auto &context = WGPUContext::getContext();
        auto device = context.getDevice();

        wgpu::SupportedLimits limits;
        device.getLimits(&limits);

        uint32_t minUniformBufferOffsetAlignment = limits.limits.minUniformBufferOffsetAlignment;

        return ceilToNextMultiple(tOriginalSize, minUniformBufferOffsetAlignment);
    }

    Uniform(size_t tSize) : Uniform(tSize, false){};

    Uniform(size_t tSize, bool tHasDynamicOffset) : mSize(tSize), mHasDynamicOffset(tHasDynamicOffset) {
        assert(mSize % 16 == 0 && "Uniform type must be a multiple of 16");
        if (!mHasDynamicOffset) {
            mBufferLengthUsed = 1;
            mStride = mSize;
        } else {
            mStride = computeStride(mSize);
        }
        resize(1);
    }

    void set(size_t tSize, void *tData) {
        set(0, mSize, tData);
    }

    void set(size_t tOffset, size_t tSize, void *tData) {
        set(0, tOffset, tSize, tData);
    }

    void set(size_t tIndex, size_t tOffset, size_t tSize, void *tData) {
        assert(mBufferLengthUsed > tIndex);

        assert(*mStorageBuffer);

        auto &context = WGPUContext::getContext();
        auto queue = context.getQueue();
        queue.writeBuffer(*mStorageBuffer, (mStride * tIndex) + tOffset, tData, tSize);
    }

    void increment() {
        assert(mHasDynamicOffset);
        mBufferLengthUsed++;

        if (mBufferLengthUsed > mBufferLength) {
            size_t newLength = mBufferLength * 2;
            resize(newLength);
        }

        assert(!(mBufferLengthUsed > mBufferLength));
    }

    void decrement() {
        assert(mHasDynamicOffset);

        mBufferLengthUsed--;

        if (mBufferLengthUsed < mBufferLength / 2 && (mBufferLength - mBufferLengthUsed) > 8) {
            size_t newLength = mBufferLength / 2;
            resize(newLength);
        }
    }

    ResourceEntry resourceEntry() const {
        ResourceEntry entry{};

        entry.bindGroupEntry.size = mSize;
        entry.bindGroupEntry.buffer = *mStorageBuffer;
        entry.bindGroupEntry.offset = 0;

        entry.bindGroupLayoutEntry.buffer.hasDynamicOffset = mHasDynamicOffset;
        entry.bindGroupLayoutEntry.buffer.type = wgpu::BufferBindingType::Uniform;
        entry.bindGroupLayoutEntry.visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
        return entry;
    }

    size_t size() const {
        return mBufferLength;
    }

    uint32_t stride() const {
        return mStride;
    }

private:
    void resize(size_t tSize) {
        wgpu::BufferDescriptor bufferDesc = wgpu::Default;
        bufferDesc.mappedAtCreation = false;
        bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform;
        bufferDesc.size = mStride * tSize;

        auto &context = WGPUContext::getContext();
        auto device = context.getDevice();

        auto newBuffer = device.createBuffer(bufferDesc);

        mStorageBuffer = newBuffer;
        mBufferLength = tSize;
    }

    size_t mBufferLengthUsed = 0;
    size_t mBufferLength = 0;
    RAIIWrapper<wgpu::Buffer> mStorageBuffer;
    bool mHasDynamicOffset;
    size_t mStride = 0;

    size_t mSize = 0;
};