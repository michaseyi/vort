#pragma once
#include "wgpu_context.hpp"

using VertexFormat = wgpu::VertexFormat;
using PrimitiveTopology = wgpu::PrimitiveTopology;
using VertexFormat = wgpu::VertexFormat;
using VertexAttribute = wgpu::VertexAttribute;

struct MeshDrawData {
    std::vector<wgpu::Buffer> vertexBuffers;
    wgpu::Buffer indexBuffer = nullptr;
    std::vector<wgpu::VertexBufferLayout> vertexBufferLayouts;
    uint64_t vertexCount;
    uint64_t instanceCount;
    wgpu::ShaderModule vertexShader = nullptr;
    wgpu::ShaderModule fragmentShader = nullptr;
    std::string vertexShaderEntry;
    std::string fragmentShaderEntry;
    PrimitiveTopology topology = PrimitiveTopology::TriangleList;
};

struct BufferEntry {
    wgpu::VertexStepMode stepMode = wgpu::VertexStepMode::VertexBufferNotUsed;
    uint32_t arrayStride;
    wgpu::VertexAttribute attribute = wgpu::Default;
    RAIIWrapper<wgpu::Buffer> buffer;
};

class RenderMeshBuffers {
public:
    inline static constexpr uint32_t VERTEX_FORMAT_TO_SIZE[] = {0,  1,  2,  3, 4, 5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15,
                                                                16, 17, 18, 4, 8, 12, 16, 23, 24, 25, 26, 27, 28, 29, 30};

    void newBuffer(VertexFormat tFormat, uint64_t tElementCount, uint32_t tAttributeLocation, bool tInstanced = false) {
        BufferEntry entry;
        entry.attribute.format = tFormat;
        entry.attribute.offset = 0;
        entry.attribute.shaderLocation = tAttributeLocation;

        if (tInstanced) {
            entry.stepMode = wgpu::VertexStepMode::Instance;
        } else {
            entry.stepMode = wgpu::VertexStepMode::Vertex;
        }

        entry.arrayStride = VERTEX_FORMAT_TO_SIZE[tFormat];

        uint64_t bufferSize = tElementCount * entry.arrayStride;  // size of tFormat;
        
        wgpu::BufferDescriptor bufferDesc = wgpu::Default;
        bufferDesc.size = bufferSize;
        bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Vertex;

        auto device = WGPUContext::getContext().getDevice();

        entry.buffer = device.createBuffer(bufferDesc);

        mBufferEntries.push_back(std::move(entry));
    }

    BufferEntry& getBufferEntry(uint32_t tAttributeLocation) {
        int32_t bufferIndex = -1;
        for (uint32_t i = 0; i < mBufferEntries.size(); i++) {
            if (mBufferEntries[i].attribute.shaderLocation == tAttributeLocation) {
                bufferIndex = i;
            }
        }

        assert(bufferIndex > -1 && "requested buffer does not exist");

        return mBufferEntries[bufferIndex];
    }

    void resizeBuffer(uint32_t tAttributeLocation, uint64_t tNewElementCount) {
        auto& bufferEntry = getBufferEntry(tAttributeLocation);

        auto device = WGPUContext::getContext().getDevice();

        auto bufferSize = tNewElementCount * bufferEntry.arrayStride;
        wgpu::BufferDescriptor bufferDesc = wgpu::Default;
        bufferDesc.size = bufferSize;
        bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Vertex;

        bufferEntry.buffer = device.createBuffer(bufferDesc);
    }

    void writeBuffer(uint32_t tAttributeLocation, uint64_t tElementOffsetIndex, uint64_t tElementCount, void* tData) {
        auto& bufferEntry = getBufferEntry(tAttributeLocation);

        auto queue = WGPUContext::getContext().getQueue();

        queue.writeBuffer(bufferEntry.buffer, tElementOffsetIndex * bufferEntry.arrayStride, tData,
                          tElementCount * bufferEntry.arrayStride);
    }

    std::vector<wgpu::VertexBufferLayout> getBufferLayouts() {
        std::vector<wgpu::VertexBufferLayout> result;
        result.reserve(mBufferEntries.size());
        for (auto& bufferEntry : mBufferEntries) {
            wgpu::VertexBufferLayout layout = wgpu::Default;
            layout.arrayStride = bufferEntry.arrayStride;
            layout.attributeCount = 1;
            layout.attributes = &bufferEntry.attribute;
            layout.stepMode = bufferEntry.stepMode;

            result.emplace_back(layout);
        }
        return result;
    }

    std::vector<wgpu::Buffer> getBuffers() {
        std::vector<wgpu::Buffer> result;
        result.reserve(mBufferEntries.size());

        for (auto& bufferEntry : mBufferEntries) {
            result.emplace_back(bufferEntry.buffer);
        }
        return result;
    }

private:
    std::vector<BufferEntry> mBufferEntries;
    std::vector<wgpu::VertexBufferLayout> mBufferLayouts;
};