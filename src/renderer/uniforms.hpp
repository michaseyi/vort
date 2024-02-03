
#pragma once

#include <cassert>

#include "resource.hpp"
#include "src/utils/raii.hpp"
#include "wgpu_context.hpp"

template <typename T>
// requires(sizeof(T) % 16 == 0)
class Uniform : public Resource {
public:
    using type = T;

    Uniform() : Uniform(false){};

    Uniform(bool tHasDynamicOffset) : mHasDynamicOffset(tHasDynamicOffset) {
        resize(1);

        if (!mHasDynamicOffset) {
            mBufferLengthUsed = 1;
        }
    }

    void set(T &tData) {
        set(0, tData);
    }

    void set(size_t tIndex, T &tData) {
        set(tIndex, 0, sizeof(T), &tData);
    }

    void set(size_t tOffset, size_t tSize, void *tData) {
        set(0, tOffset, tSize, tData);
    }

    void set(size_t tIndex, size_t tOffset, size_t tSize, void *tData) {
        assert(mBufferLengthUsed > tIndex);

        assert(*mStorageBuffer);

        auto &context = WGPUContext::getContext();
        auto queue = context.getQueue();
        queue.writeBuffer(*mStorageBuffer, (sizeof(T) * tIndex) + tOffset, tData, tSize);
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

        entry.bindGroupEntry.size = sizeof(T);
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

private:
    void resize(size_t tSize) {
        wgpu::BufferDescriptor bufferDesc = wgpu::Default;
        bufferDesc.mappedAtCreation = false;
        bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform;
        bufferDesc.size = sizeof(T) * tSize;

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
};