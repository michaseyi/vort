#include "texture.hpp"

#include "src/utils/utils.hpp"
#include "wgpu_context.hpp"
namespace renderer {

Texture::Texture(std::string path) {
  Image texture_data = load_image(path);

  wgpu::TextureDescriptor texture_desc = wgpu::Default;
  texture_desc.dimension = wgpu::TextureDimension::_2D;
  texture_desc.size = {static_cast<uint32_t>(texture_data.width),
                       static_cast<uint32_t>(texture_data.height), 1};
  texture_desc.mipLevelCount = 1;
  texture_desc.sampleCount = 1;
  texture_desc.format = wgpu::TextureFormat::RGBA8Unorm;
  texture_desc.label = path.c_str();
  texture_desc.usage =
      wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst;
  auto& context = WgpuContext::get();
  auto device = context.get_device();

  auto queue = context.get_queue();

  texture_ = device.createTexture(texture_desc);

  wgpu::ImageCopyTexture image_copy = wgpu::Default;
  image_copy.aspect = wgpu::TextureAspect::All;
  image_copy.mipLevel = 0;
  image_copy.origin = {0, 0, 0};
  image_copy.texture = *texture_;

  wgpu::TextureDataLayout data_layout = wgpu::Default;
  data_layout.bytesPerRow = texture_data.width * texture_data.channel_count;
  data_layout.rowsPerImage = texture_data.height;

  uint32_t buffer_size =
      texture_data.width * texture_data.height * texture_data.channel_count;
  queue.writeTexture(image_copy, texture_data.buffer.get(), buffer_size,
                     data_layout, texture_desc.size);
}

Texture::operator BindGroupEntries::EntryData() {
  BindGroupEntries::EntryData entry_data;
  entry_data.entry.textureView = texture_->createView();

  entry_data.layout_entry.visibility =
      wgpu::ShaderStage::Fragment | wgpu::ShaderStage::Compute;
  entry_data.layout_entry.texture.sampleType = wgpu::TextureSampleType::Float;
  entry_data.layout_entry.texture.viewDimension =
      wgpu::TextureViewDimension::_2D;
  entry_data.layout_entry.texture.multisampled = false;
  return entry_data;
}

}  // namespace renderer