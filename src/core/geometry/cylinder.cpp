#include "geometry.hpp"
#include "src/math/math.hpp"

namespace core::geometry {

Cylinder::Cylinder(float radius, float height, uint32_t segments,
                   float height_start)
    : radius_(radius),
      height_(height),
      height_start_(height_start),
      segments_(segments) {}

void Cylinder::operator()(MeshStructure& mesh_structure) {
  const float PI = math::pi<float>();

  // Create vertices for the top and bottom circles
  std::vector<MeshStructure::VertexHandle> bottom_vhandles, top_vhandles;
  for (uint32_t i = 0; i < segments_; ++i) {
    float t = i * (2 * PI) / segments_;
    float x = radius_ * math::cos(t);
    float z = radius_ * math::sin(t);

    bottom_vhandles.push_back(
        mesh_structure.add_vertex(MeshStructure::Point(x, height_start_, z)));
    top_vhandles.push_back(mesh_structure.add_vertex(
        MeshStructure::Point(x, height_ + height_start_, z)));
  }

  MeshStructure::VertexHandle bottom_center =
      mesh_structure.add_vertex(MeshStructure::Point(0, height_start_, 0));
  MeshStructure::VertexHandle top_center = mesh_structure.add_vertex(
      MeshStructure::Point(0, height_ + height_start_, 0));
  // Create faces for the top and bottom circles
  for (uint32_t i = 0; i < segments_; ++i) {
    // Bottom face
    mesh_structure.add_face(bottom_vhandles[i],
                            bottom_vhandles[(i + 1) % segments_],
                            bottom_center);
    // Top face
    mesh_structure.add_face(top_vhandles[i], top_center,
                            top_vhandles[(i + 1) % segments_]);
  }

  // Create faces for the sides of the cylinder
  for (uint32_t i = 0; i < segments_; ++i) {
    mesh_structure.add_face(bottom_vhandles[i],
                            top_vhandles[(i + 1) % segments_],
                            bottom_vhandles[(i + 1) % segments_]);

    mesh_structure.add_face(bottom_vhandles[i], top_vhandles[i],
                            top_vhandles[(i + 1) % segments_]);
  }
}
}  // namespace core::geometry