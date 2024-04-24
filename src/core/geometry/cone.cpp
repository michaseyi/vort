#include "geometry.hpp"
#include "src/math/math.hpp"

namespace core::geometry {

Cone::Cone(float radius, float height, uint32_t segments, float offset)
    : radius_(radius), height_(height), segments_(segments), offset_(offset) {}

void Cone::operator()(MeshStructure& mesh_structure) {
  auto cone_top =
      mesh_structure.add_vertex(MeshStructure::Point(0.0f, offset_ + height_, 0.0f));

  auto cone_bottom = mesh_structure.add_vertex(MeshStructure::Point(0.0f, offset_, 0.0f));

  auto two_pi = math::two_pi<float>();

  std::vector<MeshStructure::VertexHandle> circle_vhandles;

  for (uint32_t i = 0; i < segments_; i++) {
    float t = i * (two_pi / segments_);
    float x = radius_ * math::cos(t);
    float z = radius_ * math::sin(t);

    circle_vhandles.push_back(
        mesh_structure.add_vertex(MeshStructure::Point(x, offset_, z)));
  }

  std::vector<MeshStructure::VertexHandle> face_vhandles;
  face_vhandles.reserve(3);
  for (uint32_t i = 0; i < segments_; i++) {
    face_vhandles.push_back(cone_bottom);
    face_vhandles.push_back(circle_vhandles[i]);
    face_vhandles.push_back(circle_vhandles[(i + 1) % segments_]);

    mesh_structure.add_face(face_vhandles);
    face_vhandles.clear();

    face_vhandles.push_back(circle_vhandles[i]);
    face_vhandles.push_back(cone_top);
    face_vhandles.push_back(circle_vhandles[(i + 1) % segments_]);
    mesh_structure.add_face(face_vhandles);
    face_vhandles.clear();
  }
}
}  // namespace core::geometry