#pragma once

#include "bounding_volume.hpp"
#include "mesh_structure.hpp"

namespace core {

struct BVHNode {
  bool is_leaf_node;
  AABB bounding_box;
  std::array<std::shared_ptr<BVHNode>, 2> children;
  std::vector<MeshStructure::FaceHandle> primitive_list;

  BVHNode(bool is_leaf, AABB bbox);
};

class BVH {
 public:
  struct Bucket {
    int32_t primitive_count = 0;
    AABB bbox;
  };

  static AABB compute_face_bounding_box(MeshStructure& mesh_structure,
                                        MeshStructure::FaceHandle face_handle);

  static AABB union_bounding_box(AABB first, AABB second);

  static AABB compute_bounding_box(
      MeshStructure& mesh_structure,
      MeshStructure::FaceHandle face_handle = MeshStructure::FaceHandle());

  static BVH from_mesh(MeshStructure& mesh_structure);

  int32_t get_tree_height();

  void intersect(MeshStructure& mesh_structure, math::ray& ray);

  std::shared_ptr<BVHNode> root;
};

}  // namespace core