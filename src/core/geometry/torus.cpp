#include "geometry.hpp"
#include "src/math/math.hpp"

namespace core::geometry {

Torus::Torus(float major_radius, float minor_radius, uint32_t segments,
             uint32_t rings)
    : major_radius_(major_radius),
      minor_radius_(minor_radius),
      segments_(segments),
      rings_(rings) {}

void Torus::operator()(MeshStructure& mesh_structure) {
  std::vector<MeshStructure::VertexHandle> vhandles;
  vhandles.resize(segments_ * rings_);

  // mSegments -> mMinorRadius
  // mRings -> mMajorRadius

  auto two_pi = math::two_pi<float>();

  for (auto i = 0; i < rings_; i++) {
    // auto phi = (twoPI - (twoPI / static_cast<float>(mRings))) * i;

    auto phi = (two_pi / static_cast<float>(rings_)) * i;

    // math::vec3 center =
    //     (math::rotate(math::mat4{1.0f}, phi, math::vec3(0.0f, 1.0f, 0.0f))
    //     * math::vec4(1.0f, 0.0f, 0.0f, 0.0f)) * (mMajorRadius +
    //     mMinorRadius);

    math::vec3 center = (math::vec3(1.0f, 0.0f, 0.0f) *
                         (math::cos(phi) * (minor_radius_ + major_radius_))) +
                        (math::vec3(0, 0, 1.0f) *
                         (math::sin(phi) * (minor_radius_ + major_radius_)));

    auto center_direction = math::normalize(-center);

    for (auto j = 0; j < segments_; j++) {
      auto theta = (two_pi / static_cast<float>(segments_)) * j;
      // center
      auto point =
          center +
          math::vec3(center_direction * (math::cos(theta) * minor_radius_)) +
          math::vec3(math::vec3(0.0f, 1.0f, 0.0f) *
                     (math::sin(theta) * minor_radius_));

      vhandles[(i * segments_) + j] = mesh_structure.add_vertex(
          MeshStructure::Point(point[0], point[1], point[2]));
    }
  }

  for (auto i = 0; i < rings_; i++) {
    auto ii = (i + 1) % rings_;

    for (auto j = 0; j < segments_; j++) {
      auto jj = (j + 1) % segments_;

      // i,j -> ii,j -> ii,jj -> i,jj;

      mesh_structure.add_face({
          vhandles[(i * segments_) + j],
          vhandles[(ii * segments_) + j],
          vhandles[(ii * segments_) + jj],
          vhandles[(i * segments_) + jj],

      });
    }
  }
}

}  // namespace core::geometry