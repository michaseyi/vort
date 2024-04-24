#include "modifier.hpp"
namespace core::modifiers {

BevelModifier::BevelModifier(bool bevel_verices, uint32_t segment_count,
                             float width)
    : bevel_vertices_(bevel_verices),
      segment_count_(segment_count),
      width_(width){};

void BevelModifier::modify(MeshStructure& mesh_structure) {
  // auto edge = *tMesh.edges().begin();

  // auto efs = edge.faces().to_set();

  // auto v0 = edge.v0();
  // auto v1 = edge.v1();

  // auto v0duplicate0 = tMesh.add_vertex(CPUMesh::Point(0.0f));
  // auto v0duplicate1 = tMesh.add_vertex(CPUMesh::Point(0.0f));

  // auto v1duplicate0 = tMesh.add_vertex(tMesh.point(v1));
  // auto v1duplicate1 = tMesh.add_vertex(tMesh.point(v1));

  // for (auto face : v0.faces()) {
  //     if (auto found = efs.find(face); found != efs.end()) {
  //     } else {
  //         auto fvs = face.vertices().to_vector();

  //         for (auto i = fvs.begin(); i != fvs.end(); i++) {
  //             if (*i == v0) {
  //                 break;
  //             }
  //         }

  //         tMesh.delete_face(face);
  //         // tMesh.add_face(fvs)
  //     }
  // }
}

}  // namespace core::modifiers