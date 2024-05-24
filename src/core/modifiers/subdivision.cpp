#include <array>
#include <queue>
#include <tuple>
#include <unordered_map>

#include "OpenMesh/Core/Mesh/PolyMesh_ArrayKernelT.hh"
#include "OpenMesh/Tools/Subdivider/Uniform/CatmullClarkT.hh"
#include "OpenMesh/Tools/Subdivider/Uniform/LongestEdgeT.hh"
#include "OpenMesh/Tools/Subdivider/Uniform/LoopT.hh"
#include "modifier.hpp"

namespace std {
template <>
struct hash<
    std::tuple<OpenMesh::SmartVertexHandle, OpenMesh::SmartVertexHandle>> {
  size_t operator()(const std::tuple<OpenMesh::SmartVertexHandle,
                                     OpenMesh::SmartVertexHandle>& x) const {
    auto h1 = std::hash<int>{}(std::get<0>(x).idx());
    auto h2 = std::hash<int>{}(std::get<1>(x).idx());
    return h1 ^ (h2 << 1);
  }
};

}  // namespace std

namespace core::modifiers {

void loop_surface_subdivision(MeshStructure& mesh_structure, int32_t subdivision_level) {
  assert(mesh_structure.is_triangles());

  OpenMesh::Subdivider::Uniform::LoopT<MeshStructure> loop;
  loop.attach(mesh_structure);
  loop(subdivision_level);
  loop.detach();
  mesh_structure.update_face_normals();
}

void catmullClarkSurfaceSubdivision(MeshStructure& mesh_structure, int32_t subdivision_level) {
  OpenMesh::Subdivider::Uniform::CatmullClarkT<MeshStructure> catmull_clark;
  catmull_clark.attach(mesh_structure);
  catmull_clark(subdivision_level);
  catmull_clark.detach();
  mesh_structure.update_face_normals();
}

void simpleSurfaceSubdivision(MeshStructure& mesh_structure, int32_t subdivision_level) {
  std::queue<std::vector<OpenMesh::SmartVertexHandle>> face_vhandles_queue;

  for (auto face : mesh_structure.faces()) {
    auto vertices = face.vertices().to_vector();

    face_vhandles_queue.push(std::move(vertices));

    mesh_structure.delete_face(face, false);
  }

  std::vector<OpenMesh::SmartVertexHandle> vhandles;
  vhandles.reserve(6);

  while (!face_vhandles_queue.empty()) {
    auto faces_in_current_subdivision_level = face_vhandles_queue.size();
    std::unordered_map<
        std::tuple<OpenMesh::SmartVertexHandle, OpenMesh::SmartVertexHandle>,
        OpenMesh::SmartVertexHandle>
        cached_mid_points;

    for (auto i = 0; i < faces_in_current_subdivision_level; i++) {
      auto& face_vhandles = face_vhandles_queue.front();

      if (subdivision_level == 0) {
        auto face_handle = mesh_structure.add_face(face_vhandles);
        mesh_structure.set_normal(face_handle, mesh_structure.calc_face_normal(face_handle));
      } else {
        for (int j = 0, jj = 1; j < face_vhandles.size();
             j++, jj = (jj + 1) % face_vhandles.size()) {
          vhandles.emplace_back(face_vhandles[j]);

          OpenMesh::SmartVertexHandle first;
          OpenMesh::SmartVertexHandle second;

          if (face_vhandles[j].idx() < face_vhandles[jj].idx()) {
            first = face_vhandles[j];
            second = face_vhandles[jj];
          } else {
            first = face_vhandles[jj];
            second = face_vhandles[j];
          }

          OpenMesh::SmartVertexHandle mid_point;

          if (auto cached_mid_point =
                  cached_mid_points.find(std::tuple(first, second));
              cached_mid_point != cached_mid_points.end()) {
            mid_point = cached_mid_point->second;

            cached_mid_points.erase(cached_mid_point);
          } else {
            mid_point = mesh_structure.add_vertex(
                (mesh_structure.point(first) + mesh_structure.point(second)) / 2.0f);
            cached_mid_points[std::tuple(first, second)] = mid_point;
          }

          vhandles.emplace_back(mid_point);
        }
        face_vhandles_queue.push({vhandles[0], vhandles[1], vhandles[5]});
        face_vhandles_queue.push({vhandles[1], vhandles[3], vhandles[5]});
        face_vhandles_queue.push({vhandles[1], vhandles[2], vhandles[3]});
        face_vhandles_queue.push({vhandles[3], vhandles[4], vhandles[5]});

        vhandles.clear();
      }
      face_vhandles_queue.pop();
    }
    subdivision_level--;
  }
}

using SurfaceSubdivisionAlgorithm = decltype(loop_surface_subdivision)*;

static std::array<SurfaceSubdivisionAlgorithm, 3> surfaceSubdivisionAlgorithms =
    {simpleSurfaceSubdivision, loop_surface_subdivision,
     catmullClarkSurfaceSubdivision};

void SubdivisionModifier::modify(MeshStructure& mesh_structure) {
  surfaceSubdivisionAlgorithms[static_cast<int>(subdivision_algorithm_)](
       mesh_structure , subdivision_level_);

  mesh_structure.garbage_collection();
}
}  // namespace core::modifiers
