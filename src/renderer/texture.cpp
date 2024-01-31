#include "texture.hpp"

#include "src/utils/utils.hpp"

Texture Texture::create2DTexture(std::string tPath) {
    Image textureData = loadImage(tPath);

    wgpu::TextureDescriptor textureDesc = wgpu::Default;
    textureDesc.dimension = wgpu::TextureDimension::_2D;
    textureDesc.size = {static_cast<uint32_t>(textureData.width), static_cast<uint32_t>(textureData.height),
                        1};
    textureDesc.mipLevelCount = 1;
    textureDesc.sampleCount = 1;
    textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;

    textureDesc.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst;

    textureDesc.viewFormatCount = 0;
    textureDesc.viewFormats = nullptr;
}