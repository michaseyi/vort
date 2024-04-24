#include "geometry.hpp"
#include "src/math/math.hpp"

namespace core::geometry {

UvSphere::UvSphere(uint32_t longitude_segments, uint32_t latitude_segments,
                   float radius)
    : longitude_segments_(longitude_segments),
      latitude_segments_(latitude_segments),
      radius_(radius){};

void UvSphere::operator()(MeshStructure& mesh_structure) {
  float pi = math::pi<float>();

  std::vector<std::vector<MeshStructure::VertexHandle>> vhandles;

  for (uint32_t i = 0; i <= longitude_segments_; ++i) {
    vhandles.push_back({});
    float theta = i * 2 * pi / longitude_segments_;

    for (uint32_t j = 0; j <= latitude_segments_; ++j) {
      float phi = (j * pi / latitude_segments_) - (pi / 2.0f);
      float x = radius_ * math::cos(phi) * math::cos(-theta);
      float z = radius_ * math::cos(phi) * math::sin(-theta);
      float y = radius_ * math::sin(phi);
      vhandles[i].push_back(
          mesh_structure.add_vertex(MeshStructure::Point(x, y, z)));
      mesh_structure.set_texcoord2D(
          vhandles[i].back(),
          MeshStructure::TexCoord2D(i / (float)longitude_segments_,
                                    j / (float)latitude_segments_));
    }
  }

  // auto topCenter = tMesh.add_vertex(
  //     MeshStructure::Point(mRadius * math::cos(math::radians(90.0f)),

  //                          mRadius * math::sin(math::radians(90.0f)),
  //                          0.0f));

  // tMesh.set_texcoord2D(topCenter, MeshStructure::TexCoord2D(0.5f, 0.0f));
  // auto bottomCenter = tMesh.add_vertex(
  //     MeshStructure::Point(mRadius * math::cos(math::radians(90.0f)),
  //                          mRadius * math::sin(math::radians(-90.0f)),
  //                          0.0f));

  // tMesh.set_texcoord2D(bottomCenter,
  // MeshStructure::TexCoord2D(0.5f, 1.0f));
  std::vector<MeshStructure::VertexHandle> face_vhandles;
  // faceVhandles.reserve(3);

  // for (uint32_t i = 0; i < mLongitudeSegments; ++i) {
  //   auto max_index = vHandles[i].size() - 1;
  //   faceVhandles.push_back(vHandles[i][max_index]);
  //   faceVhandles.push_back(vHandles[(i + 1) %
  //   mLongitudeSegments][max_index]); faceVhandles.push_back(topCenter);
  //   tMesh.add_face(faceVhandles);
  //   faceVhandles.clear();

  //   faceVhandles.push_back(vHandles[i][0]);
  //   faceVhandles.push_back(bottomCenter);
  //   faceVhandles.push_back(vHandles[(i + 1) % mLongitudeSegments][0]);
  //   tMesh.add_face(faceVhandles);
  //   faceVhandles.clear();
  // }

  // Create faces
  face_vhandles.reserve(4);
  for (uint32_t i = 0; i < vhandles.size() - 1; ++i) {
    for (uint32_t j = 0; j < vhandles[0].size() - 1; ++j) {
      // uint32_t nextI = (i + 1) % mLongitudeSegments;
      uint32_t next_i = (i + 1);
      int next_j = (j + 1);

      face_vhandles.push_back(vhandles[i][j]);
      face_vhandles.push_back(vhandles[next_i][j]);
      face_vhandles.push_back(vhandles[next_i][next_j]);
      face_vhandles.push_back(vhandles[i][next_j]);

      mesh_structure.add_face(face_vhandles);
      face_vhandles.clear();
    }
  }
}
}  // namespace core::geometry