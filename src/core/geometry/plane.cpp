
#include "geometry.hpp"
namespace core::geometry {

Plane::Plane(float width, float height) : width_(width), height_(height) {}

void Plane::operator()(MeshStructure& mesh_structure) {
  float half_width = width_ / 2.0f;
  float half_height = height_ / 2.0f;

  std::vector<MeshStructure::VertexHandle> vhandles = {
      mesh_structure.add_vertex(
          MeshStructure::Point(-half_width, 0.0f, half_height)),
      mesh_structure.add_vertex(
          MeshStructure::Point(half_width, 0.0f, half_height)),
      mesh_structure.add_vertex(
          MeshStructure::Point(half_width, 0.0f, -half_height)),
      mesh_structure.add_vertex(
          MeshStructure::Point(-half_width, 0.0f, -half_height)),
  };

  mesh_structure.set_texcoord2D(vhandles[0],
                                MeshStructure::TexCoord2D(0.0f, 0.0f));
  mesh_structure.set_texcoord2D(vhandles[1],
                                MeshStructure::TexCoord2D(0.0f, 1.0f));
  mesh_structure.set_texcoord2D(vhandles[2],
                                MeshStructure::TexCoord2D(1.0f, 1.0f));
  mesh_structure.set_texcoord2D(vhandles[3],
                                MeshStructure::TexCoord2D(1.0f, 0.0f));

  mesh_structure.add_face({vhandles[0], vhandles[1], vhandles[2], vhandles[3]});
}

}  // namespace core::geometry