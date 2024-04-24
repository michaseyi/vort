#include "geometry.hpp"
namespace core::geometry {

CombinedGeometry::CombinedGeometry(Geometry* a, Geometry* b) {
  geometries_.emplace_back(a);
  geometries_.emplace_back(b);
}

void CombinedGeometry::operator()(MeshStructure& mesh_structure) {
  for (auto& geometry : geometries_) {
    (*geometry)(mesh_structure);
  }
}
}  // namespace core::geometry