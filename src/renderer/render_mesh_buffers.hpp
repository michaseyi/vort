#pragma once
#include "wgpu_context.hpp"
namespace renderer {

using VertexFormat = wgpu::VertexFormat;
using PrimitiveTopology = wgpu::PrimitiveTopology;
using VertexFormat = wgpu::VertexFormat;
using VertexAttribute = wgpu::VertexAttribute;

struct RenderObject {
  std::vector<wgpu::Buffer> vertex_buffers;
  wgpu::Buffer index_buffer = nullptr;
  std::vector<wgpu::VertexBufferLayout> vertex_buffer_layouts;
  uint64_t vertex_count;
  uint64_t instance_count;
  wgpu::ShaderModule vertex_shader = nullptr;
  wgpu::ShaderModule fragment_shader = nullptr;
  std::string vertex_shader_entry;
  std::string fragment_shader_entry;
  PrimitiveTopology topology = PrimitiveTopology::TriangleList;
};

struct BufferEntry {
  wgpu::VertexStepMode step_mode = wgpu::VertexStepMode::VertexBufferNotUsed;
  uint32_t array_stride;
  wgpu::VertexAttribute attribute = wgpu::Default;
  RaiiWrapper<wgpu::Buffer> buffer;
};

class RenderMeshBuffers {
 public:
  inline static constexpr uint32_t kVertexFormatToSize[] = {
      0,  1,  2,  3, 4, 5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15,
      16, 17, 18, 4, 8, 12, 16, 23, 24, 25, 26, 27, 28, 29, 30};

  void new_buffer(VertexFormat format, uint64_t element_count,
                  uint32_t attribute_location, bool instanced = false);

  BufferEntry& get_buffer_entry(uint32_t attribute_location);

  void resize_buffer(uint32_t attribute_location, uint64_t new_element_count);

  void write_buffer(uint32_t attribute_location, uint64_t element_offset_index,
                    uint64_t element_count, void* data);

  std::vector<wgpu::VertexBufferLayout> get_buffer_layouts();

  std::vector<wgpu::Buffer> get_buffers();

 private:
  std::vector<BufferEntry> buffer_entries_;
  std::vector<wgpu::VertexBufferLayout> buffer_layouts_;
};
}  // namespace renderer