#pragma once
#include <memory>

#include "geometry/geometry.hpp"
#include "mesh_structure.hpp"
#include "src/math/math.hpp"
#include "src/renderer/materials/material.hpp"
#include "src/renderer/render_mesh_buffers.hpp"

namespace core {

class Mesh {
 public:
  std::unique_ptr<renderer::materials::Material> material;
  std::unique_ptr<geometry::Geometry> geometry;

  Mesh() = default;

  Mesh(geometry::Geometry* geometry, renderer::materials::Material* material);

  void update_triangel_count();

  void shade_smooth();
  void shade_normal();

  void apply_transformation(math::mat4 transform_matrix);

  void update_gpu_data();

  uint32_t get_triangle_count();

  renderer::RenderObject get_face_render_object();
  renderer::RenderObject get_edge_render_object();
  renderer::RenderObject get_vertex_render_object();

  void update_face_gpu_data();

  void update_edge_gpu_data();

  void update_vertex_gpu_data();

  MeshStructure& get_structure();

  bool shaded_smooth();

 private:
  renderer::RenderMeshBuffers face_buffers_;
  renderer::RenderMeshBuffers edge_buffers_;
  renderer::RenderMeshBuffers vertex_buffers_;

  bool face_buffers_need_update_ = true;
  bool edge_buffers_need_update_ = true;
  bool vertex_buffers_need_update_ = true;

  MeshStructure mesh_structure_;

  inline static std::unique_ptr<renderer::Shader> shader_;
  inline static bool initialized_ = false;
  bool shaded_smooth_ = false;

  uint32_t triangle_count_ = 0;

  static void initialize_shader();
};
}  // namespace core