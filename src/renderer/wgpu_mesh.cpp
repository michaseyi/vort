

#include "wgpu_mesh.hpp"

using namespace wgpu;

VertexBufferLayout WGPUMesh::description() {
    VertexBufferLayout layout = Default;
    layout.stepMode = VertexStepMode::Vertex;
    layout.attributeCount = mAttributes.size();
    layout.attributes = mAttributes.data();
    layout.arrayStride = attributeSize();

    return layout;
}

uint64_t WGPUMesh::attributeSize() {
    uint64_t size = 0;
    for (auto &attribute : mAttributes) {
        size += VertexFormatToSize[static_cast<int32_t>(attribute.format)];
    }
    return size;
}
void WGPUMesh::setAttribute(VertexFormat tFormat, uint64_t tOffset, uint32_t tShaderLocation) {
    VertexAttribute vertexAttribute = Default;
    vertexAttribute.format = tFormat;
    vertexAttribute.offset = tOffset;
    vertexAttribute.shaderLocation = tShaderLocation;
    mAttributes.push_back(vertexAttribute);
}

void WGPUMesh::writeVertexBuffer(void *tData, size_t tSize) {
    auto &context = WGPUContext::getContext();
    context.getQueue().writeBuffer(mVertexBuffer, 0, tData, tSize);
}
void WGPUMesh::writeIndexBuffer(void *tData, size_t tSize) {
    auto &context = WGPUContext::getContext();
    context.getQueue().writeBuffer(mIndexBuffer, 0, tData, tSize);
}

void WGPUMesh::allocateIndexBuffer(size_t tSize) {
    if (*mIndexBuffer && mIndexBuffer->getSize() != tSize) {
        return;
    }
    auto &context = WGPUContext::getContext();

    BufferDescriptor indexBufferDesc = Default;
    indexBufferDesc.mappedAtCreation = false;
    indexBufferDesc.size = tSize;
    indexBufferDesc.usage = BufferUsage::Index | BufferUsage::CopyDst;
    mIndexBuffer = context.getDevice().createBuffer(indexBufferDesc);
}
void WGPUMesh::allocateVertexBuffer(size_t tSize) {
    if (*mVertexBuffer && mVertexBuffer->getSize() == tSize) {
        return;
    }

    auto &context = WGPUContext::getContext();

    BufferDescriptor vertexBufferDesc = Default;
    vertexBufferDesc.mappedAtCreation = false;
    vertexBufferDesc.size = tSize;
    vertexBufferDesc.usage = BufferUsage::Vertex | BufferUsage::CopyDst;

    mVertexBuffer = context.getDevice().createBuffer(vertexBufferDesc);
}