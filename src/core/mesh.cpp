#include "mesh.hpp"

#include <memory>

namespace core {

Mesh::Mesh(geometry::Geometry* geometry,
           renderer::materials::Material* material)
    : geometry(geometry), material(material) {
  initialize_shader();
  mesh_structure_.request_vertex_normals();
  mesh_structure_.request_vertex_texcoords2D();
  mesh_structure_.request_face_normals();
  mesh_structure_.request_face_status();
  mesh_structure_.request_halfedge_status();
  mesh_structure_.request_edge_status();
  mesh_structure_.request_vertex_status();

  (*geometry)(mesh_structure_);

  mesh_structure_.update_normals();

  update_triangel_count();

  face_buffers_.new_buffer(renderer::VertexFormat::Float32x3,
                           triangle_count_ * 3,
                           0);  // position
  face_buffers_.new_buffer(renderer::VertexFormat::Float32x3,
                           triangle_count_ * 3,
                           1);  // color
  face_buffers_.new_buffer(renderer::VertexFormat::Float32x3,
                           triangle_count_ * 3,
                           2);  // normal
  face_buffers_.new_buffer(renderer::VertexFormat::Float32x2,
                           triangle_count_ * 3,
                           3);  // uv

  edge_buffers_.new_buffer(renderer::VertexFormat::Float32x3,
                           mesh_structure_.n_edges() * 2,
                           0);  // position
  edge_buffers_.new_buffer(renderer::VertexFormat::Float32x3,
                           mesh_structure_.n_edges() * 2,
                           1);  // color

  vertex_buffers_.new_buffer(renderer::VertexFormat::Float32x3,
                             mesh_structure_.n_vertices(), 0,
                             true);  // position
  vertex_buffers_.new_buffer(renderer::VertexFormat::Float32x3,
                             mesh_structure_.n_vertices(), 1,
                             true);  // color

  update_gpu_data();
}

void Mesh::update_triangel_count() {
  triangle_count_ = 0;
  for (auto face : mesh_structure_.faces()) {
    triangle_count_ += face.valence() - 2;
  }
}

void Mesh::shade_smooth() {
  if (!shaded_smooth_) {
    shaded_smooth_ = true;
    face_buffers_need_update_ = true;
  }
}
void Mesh::shade_normal() {
  if (shaded_smooth_) {
    shaded_smooth_ = false;
    face_buffers_need_update_ = true;
  }
}

void Mesh::apply_transformation(math::mat4 transform_matrix) {
  for (auto vertex : mesh_structure_.vertices()) {
    auto& point = mesh_structure_.point(vertex);
    auto new_point =
        transform_matrix * math::vec4(point[0], point[1], point[2], 1.0f);
    point[0] = new_point.x;
    point[1] = new_point.y;
    point[2] = new_point.z;
  }

  mesh_structure_.update_normals();

  edge_buffers_need_update_ = true;
  face_buffers_need_update_ = true;
  vertex_buffers_need_update_ = true;
}

void Mesh::update_gpu_data() {
  update_face_gpu_data();
  update_edge_gpu_data();
  update_vertex_gpu_data();
}

uint32_t Mesh::get_triangle_count() {
  return triangle_count_;
}

renderer::RenderObject Mesh::get_face_render_object() {
  renderer::RenderObject data;
  data.vertex_buffers = face_buffers_.get_buffers();
  data.vertex_buffer_layouts = face_buffers_.get_buffer_layouts();
  data.vertex_count = triangle_count_ * 3;
  data.instance_count = 1;
  data.vertex_shader = shader_->shader_module;
  data.fragment_shader = shader_->shader_module;
  data.vertex_shader_entry = "vs_main_face";
  data.fragment_shader_entry = "fs_main_face";
  data.topology = renderer::PrimitiveTopology::TriangleList;
  return data;
}
renderer::RenderObject Mesh::get_edge_render_object() {
  renderer::RenderObject data;
  data.vertex_buffers = edge_buffers_.get_buffers();
  data.vertex_buffer_layouts = edge_buffers_.get_buffer_layouts();
  data.vertex_count = mesh_structure_.n_edges() * 2;
  data.instance_count = 1;
  data.vertex_shader = shader_->shader_module;
  data.fragment_shader = shader_->shader_module;
  data.vertex_shader_entry = "vs_main_edge";
  data.fragment_shader_entry = "fs_main_edge";
  data.topology = renderer::PrimitiveTopology::LineList;
  return data;
}
renderer::RenderObject Mesh::get_vertex_render_object() {
  renderer::RenderObject data;
  data.vertex_buffers = vertex_buffers_.get_buffers();
  data.vertex_buffer_layouts = vertex_buffers_.get_buffer_layouts();
  data.vertex_count = 6;
  data.instance_count = mesh_structure_.n_vertices();
  data.vertex_shader = shader_->shader_module;
  data.fragment_shader = shader_->shader_module;
  data.vertex_shader_entry = "vs_main_point";
  data.fragment_shader_entry = "fs_main_point";
  data.topology = renderer::PrimitiveTopology::TriangleList;
  return data;
}

void Mesh::update_face_gpu_data() {
  if (!face_buffers_need_update_) {
    return;
  }
  face_buffers_need_update_ = false;

  std::vector<math::vec3> positions;
  std::vector<math::vec3> colors;
  std::vector<math::vec3> normals;
  std::vector<math::vec2> uvs;

  auto size = triangle_count_ * 3;
  positions.reserve(size);
  normals.reserve(size);
  colors.reserve(size);
  uvs.reserve(size);

  math::vec3 color{0.5};
  for (auto face : mesh_structure_.faces()) {
    auto normal = mesh_structure_.normal(face);

    auto face_vhandles = face.vertices().to_vector();

    auto start_vertex_handle = face_vhandles[0];
    auto start_vertex_point = mesh_structure_.point(start_vertex_handle);
    auto start_vertex_tex_coord =
        mesh_structure_.texcoord2D(start_vertex_handle);

    for (auto i = 1, ii = i + 1; i < face_vhandles.size() - 1; i++, ii++) {
      if (shaded_smooth_) {
        normal = mesh_structure_.normal(start_vertex_handle);
      }
      positions.emplace_back(start_vertex_point[0], start_vertex_point[1],
                             start_vertex_point[2]);
      normals.emplace_back(normal[0], normal[1], normal[2]);
      colors.emplace_back(color[0], color[1], color[2]);
      uvs.emplace_back(start_vertex_tex_coord[0], start_vertex_tex_coord[1]);

      auto point = mesh_structure_.point(face_vhandles[i]);
      auto tex_coord = mesh_structure_.texcoord2D(face_vhandles[i]);
      if (shaded_smooth_) {
        normal = mesh_structure_.normal(face_vhandles[i]);
      }

      positions.emplace_back(point[0], point[1], point[2]);
      normals.emplace_back(normal[0], normal[1], normal[2]);
      colors.emplace_back(color[0], color[1], color[2]);
      uvs.emplace_back(tex_coord[0], tex_coord[1]);

      point = mesh_structure_.point(face_vhandles[ii]);
      tex_coord = mesh_structure_.texcoord2D(face_vhandles[ii]);

      if (shaded_smooth_) {
        normal = mesh_structure_.normal(face_vhandles[ii]);
      }
      positions.emplace_back(point[0], point[1], point[2]);
      normals.emplace_back(normal[0], normal[1], normal[2]);
      colors.emplace_back(color[0], color[1], color[2]);
      uvs.emplace_back(tex_coord[0], tex_coord[1]);
    }
  }

  face_buffers_.write_buffer(0, 0, size, positions.data());
  face_buffers_.write_buffer(1, 0, size, colors.data());
  face_buffers_.write_buffer(2, 0, size, normals.data());
  face_buffers_.write_buffer(3, 0, size, uvs.data());
}

void Mesh::update_edge_gpu_data() {
  if (!edge_buffers_need_update_) {
    return;
  }
  edge_buffers_need_update_ = false;

  std::vector<math::vec3> positions;
  std::vector<math::vec3> colors;

  auto size = mesh_structure_.n_edges() * 2;

  positions.reserve(size);
  colors.reserve(size);

  math::vec3 color{0.65f};
  for (auto edge : mesh_structure_.edges()) {
    for (auto vertex : edge.vertices()) {
      auto point = mesh_structure_.point(vertex);

      positions.emplace_back(point[0], point[1], point[2]);
      colors.emplace_back(color[0], color[1], color[2]);
    }
  }

  edge_buffers_.write_buffer(0, 0, size, positions.data());
  edge_buffers_.write_buffer(1, 0, size, colors.data());
}

void Mesh::update_vertex_gpu_data() {
  if (!vertex_buffers_need_update_) {
    return;
  }

  vertex_buffers_need_update_ = false;

  std::vector<math::vec3> positions;
  std::vector<math::vec3> colors;

  auto size = mesh_structure_.n_vertices();

  positions.reserve(size);
  colors.reserve(size);

  math::vec3 color{0.0f};
  for (auto vertex : mesh_structure_.vertices()) {
    auto point = mesh_structure_.point(vertex);
    positions.emplace_back(point[0], point[1], point[2]);
    colors.emplace_back(color[0], color[1], color[2]);
  }

  vertex_buffers_.write_buffer(0, 0, size, positions.data());
  vertex_buffers_.write_buffer(1, 0, size, colors.data());
}

MeshStructure& Mesh::get_structure() {
  return mesh_structure_;
}

bool Mesh::shaded_smooth() {
  return shaded_smooth_;
}

void Mesh::initialize_shader() {
  if (!initialized_) {
    shader_.reset(new renderer::Shader("mesh.wgsl"));
    initialized_ = true;
  }
}
}  // namespace core