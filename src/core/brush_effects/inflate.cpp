
#include <unordered_set>
#include <vector>

#include "../timer.hpp"
#include "brush_effect.hpp"

namespace core::brush_effects {

void InflateBrush::apply(Brush& brush, MeshStructure& mesh_structure) {

  Timer timer;
  timer.start();

  std::vector<MeshStructure::VertexHandle> vhandles_to_update;
  std::unordered_set<MeshStructure::FaceHandle> fhandles_to_update;

  for (auto vertex : mesh_structure.vertices()) {
    auto& point = mesh_structure.point(vertex);
    auto& normal = mesh_structure.normal(vertex);

    if (math::dot(-brush.direction,
                  math::vec3(normal[0], normal[1], normal[2])) < 0.0f) {
      continue;
    }

    auto vertex_disance_to_line =
        math::length(math::cross(
            (brush.position - math::vec3(point[0], point[1], point[2])),
            brush.direction)) /
        math::length(brush.direction);

    if (vertex_disance_to_line > brush.radius) {
      continue;
    }

    point += normal * 0.001 *
             math::sin((1 - vertex_disance_to_line / brush.radius) *
                       math::half_pi<float>());

    vhandles_to_update.emplace_back(vertex);

    for (auto face : vertex.faces()) {
      fhandles_to_update.emplace(face);
    }
  }

  std::cout << "Mesh Update CPU Time Before Mesh Normal Update: "
            << timer.elapsed_ms() << "ms \n";

  timer.reset();

  for (auto& face : fhandles_to_update) {
    mesh_structure.update_normal(face);
  }

  for (auto& vertex : vhandles_to_update) {
    mesh_structure.update_normal(vertex);
  }
  std::cout << "Mesh Update CPU Time: " << timer.elapsed_ms() << "ms \n";
}
}  // namespace core::brush_effects