#pragma once
#include "src/utils/raii.hpp"
#include "wgpu_context.hpp"

using VertexFormat = wgpu::VertexFormat;

struct MeshDrawData {
    std::vector<wgpu::Buffer> vertexBuffers;
    wgpu::Buffer indexBuffer = nullptr;
    std::vector<wgpu::VertexBufferLayout> vertexBufferLayouts;
    uint64_t vertexCount;
};

class WGPUMesh {
    // TODO: Complete all the mappings. For now only Float32 to Float32X4 works.
    inline static constexpr uint32_t VertexFormatToSize[] = {0,  1,  2,  3,  4,  5,  6,  7,  8, 9, 10,
                                                             11, 12, 13, 14, 15, 16, 17, 18, 4, 8, 12,
                                                             16, 23, 24, 25, 26, 27, 28, 29, 30};

    friend class Mesh;

public:
    wgpu::VertexBufferLayout description();

    void setAttribute(wgpu::VertexFormat tFormat, uint64_t tOffset, uint32_t tShaderLocation);

    void writeVertexBuffer(void *tData, size_t tSize);

    void writeIndexBuffer(void *tData, size_t tSize);

    void allocateIndexBuffer(size_t tSize);

    void allocateVertexBuffer(size_t tSize);

private:
    uint64_t attributeSize();

private:
    std::vector<wgpu::VertexAttribute> mAttributes;
    RAIIWrapper<wgpu::Buffer> mIndexBuffer;
    RAIIWrapper<wgpu::Buffer> mVertexBuffer;
};
