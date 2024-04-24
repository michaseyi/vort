
#include "render_mesh_buffers.hpp"

namespace renderer {

void RenderMeshBuffers::new_buffer(VertexFormat format, uint64_t element_count,
                                   uint32_t attribute_location,
                                   bool instanced) {
  BufferEntry entry;
  entry.attribute.format = format;
  entry.attribute.offset = 0;
  entry.attribute.shaderLocation = attribute_location;

  if (instanced) {
    entry.step_mode = wgpu::VertexStepMode::Instance;
  } else {
    entry.step_mode = wgpu::VertexStepMode::Vertex;
  }

  entry.array_stride = kVertexFormatToSize[format];

  uint64_t buffer_size =
      element_count * entry.array_stride;  // size of tFormat;

  wgpu::BufferDescriptor buffer_desc = wgpu::Default;
  buffer_desc.size = buffer_size;
  buffer_desc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Vertex;

  auto device = WgpuContext::get().get_device();

  entry.buffer = device.createBuffer(buffer_desc);

  buffer_entries_.push_back(std::move(entry));
}

BufferEntry& RenderMeshBuffers::get_buffer_entry(uint32_t attribute_location) {
  int32_t buffer_index = -1;
  for (uint32_t i = 0; i < buffer_entries_.size(); i++) {
    if (buffer_entries_[i].attribute.shaderLocation == attribute_location) {
      buffer_index = i;
    }
  }

  assert(buffer_index > -1 && "requested buffer does not exist");

  return buffer_entries_[buffer_index];
}

void RenderMeshBuffers::resize_buffer(uint32_t attribute_location,
                                      uint64_t new_element_count) {
  auto& buffer_entry = get_buffer_entry(attribute_location);

  auto device = WgpuContext::get().get_device();

  auto buffer_size = new_element_count * buffer_entry.array_stride;
  wgpu::BufferDescriptor buffer_desc = wgpu::Default;
  buffer_desc.size = buffer_size;
  buffer_desc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Vertex;

  buffer_entry.buffer = device.createBuffer(buffer_desc);
}

void RenderMeshBuffers::write_buffer(uint32_t attribute_location,
                                     uint64_t element_offset_index,
                                     uint64_t element_count, void* data) {
  auto& buffer_entry = get_buffer_entry(attribute_location);

  auto queue = WgpuContext::get().get_queue();

  queue.writeBuffer(buffer_entry.buffer,
                    element_offset_index * buffer_entry.array_stride, data,
                    element_count * buffer_entry.array_stride);
}

std::vector<wgpu::VertexBufferLayout> RenderMeshBuffers::get_buffer_layouts() {
  std::vector<wgpu::VertexBufferLayout> result;
  result.reserve(buffer_entries_.size());
  for (auto& buffer_entry : buffer_entries_) {
    wgpu::VertexBufferLayout layout = wgpu::Default;
    layout.arrayStride = buffer_entry.array_stride;
    layout.attributeCount = 1;
    layout.attributes = &buffer_entry.attribute;
    layout.stepMode = buffer_entry.step_mode;

    result.emplace_back(layout);
  }
  return result;
}

std::vector<wgpu::Buffer> RenderMeshBuffers::get_buffers() {
  std::vector<wgpu::Buffer> result;
  result.reserve(buffer_entries_.size());

  for (auto& buffer_entry : buffer_entries_) {
    result.emplace_back(buffer_entry.buffer);
  }
  return result;
}

}  // namespace renderer