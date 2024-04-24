#include <limits>
#include <sstream>

#include "geometry.hpp"
#include "src/math/math.hpp"
#include "src/utils/utils.hpp"
namespace core::geometry {

ObjLoader::ObjLoader(std::string obj_path) : obj_path_(obj_path){};

void ObjLoader::operator()(MeshStructure& mesh_structure) {
  std::stringstream ss(load_string(obj_path_));
  std::string line_buffer;

  std::vector<MeshStructure::VertexHandle> face_vertices;
  face_vertices.reserve(3);
  while (!ss.eof()) {
    std::getline(ss, line_buffer);
    if (line_buffer.starts_with("v")) {
      std::stringstream line(line_buffer);
      MeshStructure::Point point;
      line.ignore(2);
      line >> point[0] >> point[1] >> point[2];
      mesh_structure.add_vertex(point);
    } else if (line_buffer.starts_with("f")) {
      std::stringstream line(line_buffer);
      line.ignore(2);
      int32_t vertexIdx;
      line >> vertexIdx;
      face_vertices.emplace_back(vertexIdx - 1);

      line.ignore(std::numeric_limits<std::streamsize>::max(), ' ');
      line >> vertexIdx;
      face_vertices.emplace_back(vertexIdx - 1);

      line.ignore(std::numeric_limits<std::streamsize>::max(), ' ');
      line >> vertexIdx;
      face_vertices.emplace_back(vertexIdx - 1);

      mesh_structure.add_face(face_vertices);
      face_vertices.clear();
    }
  }
}
}  // namespace core::geometry