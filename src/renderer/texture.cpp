#include "texture.hpp"

#include "src/utils/utils.hpp"
#include "wgpu_context.hpp"

Texture Texture::create2DTexture(std::string tPath) {
    Image textureData = loadImage(tPath);
    Texture texture;

    wgpu::TextureDescriptor textureDesc = wgpu::Default;
    textureDesc.dimension = wgpu::TextureDimension::_2D;
    textureDesc.size = {static_cast<uint32_t>(textureData.width), static_cast<uint32_t>(textureData.height),
                        1};
    textureDesc.mipLevelCount = 1;
    textureDesc.sampleCount = 1;
    textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;

    textureDesc.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst;

    auto &context = WGPUContext::getContext();
    auto device = context.getDevice();

    auto queue = context.getQueue();

    texture.mWGPUTexture = device.createTexture(textureDesc);

    wgpu::ImageCopyTexture imageCopy = wgpu::Default;
    imageCopy.aspect = wgpu::TextureAspect::All;
    imageCopy.mipLevel = 0;
    imageCopy.origin = {0, 0, 0};
    imageCopy.texture = *texture.mWGPUTexture;

    wgpu::TextureDataLayout dataLayout = wgpu::Default;
    dataLayout.bytesPerRow = textureData.width * textureData.channelCount;
    dataLayout.rowsPerImage = textureData.height;

    uint32_t bufferSize = textureData.width * textureData.height * textureData.channelCount;
    queue.writeTexture(imageCopy, textureData.buffer.get(), bufferSize, dataLayout, textureDesc.size);

    return texture;
}