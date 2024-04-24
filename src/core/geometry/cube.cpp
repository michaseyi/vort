
#include "geometry.hpp"
namespace core::geometry {

Cube::Cube(float tSize) : size_(tSize) {}

void Cube::operator()(MeshStructure& mesh_structure) {
  float h = size_ * 0.5;
  std::vector<MeshStructure::VertexHandle> vhandles;
  vhandles.reserve(8);
  vhandles.push_back(
      mesh_structure.add_vertex(MeshStructure::Point(-h, -h, h)));
  vhandles.push_back(mesh_structure.add_vertex(MeshStructure::Point(h, -h, h)));
  vhandles.push_back(mesh_structure.add_vertex(MeshStructure::Point(h, h, h)));
  vhandles.push_back(mesh_structure.add_vertex(MeshStructure::Point(-h, h, h)));
  vhandles.push_back(
      mesh_structure.add_vertex(MeshStructure::Point(-h, -h, -h)));
  vhandles.push_back(
      mesh_structure.add_vertex(MeshStructure::Point(h, -h, -h)));
  vhandles.push_back(mesh_structure.add_vertex(MeshStructure::Point(h, h, -h)));
  vhandles.push_back(
      mesh_structure.add_vertex(MeshStructure::Point(-h, h, -h)));

  mesh_structure.add_face({vhandles[1], vhandles[5], vhandles[6], vhandles[2]});
  mesh_structure.add_face({vhandles[0], vhandles[1], vhandles[2], vhandles[3]});
  mesh_structure.add_face({vhandles[4], vhandles[0], vhandles[3], vhandles[7]});
  mesh_structure.add_face({vhandles[7], vhandles[6], vhandles[5], vhandles[4]});
  mesh_structure.add_face({vhandles[2], vhandles[6], vhandles[7], vhandles[3]});
  mesh_structure.add_face({vhandles[4], vhandles[5], vhandles[1], vhandles[0]});
}
}  // namespace core::geometry