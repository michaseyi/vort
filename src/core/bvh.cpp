#include "bvh.hpp"

#include <queue>
#include <stack>

#include "timer.hpp"
namespace core {

BVHNode::BVHNode(bool is_leaf_node, AABB bbox)
    : is_leaf_node(is_leaf_node), bounding_box(bbox) {}

AABB BVH::compute_face_bounding_box(MeshStructure& mesh_structure,
                                    MeshStructure::FaceHandle face_handle) {
  return compute_bounding_box(mesh_structure, face_handle);
}

AABB BVH::union_bounding_box(AABB first, AABB second) {
  math::vec3 min = first.bounds[0];
  math::vec3 max = first.bounds[1];

  for (auto i = 0; i < 3; i++) {
    if (second.bounds[1][i] > max[i]) {
      max[i] = second.bounds[1][i];
    }

    if (second.bounds[0][i] < min[i]) {
      min[i] = second.bounds[0][i];
    }
  }

  return AABB(min, max);
}

AABB BVH::compute_bounding_box(MeshStructure& mesh_structure,
                               MeshStructure::FaceHandle face_handle) {
  math::vec3 min(0.0f);
  math::vec3 max(0.0f);

  bool is_min_max_initialized = false;

  auto a = [&](MeshStructure::VertexHandle vertex) {
    auto& point = mesh_structure.point(vertex);
    if (!is_min_max_initialized) {
      min.x = point[0];
      min.y = point[1];
      min.z = point[2];

      max.x = point[0];
      max.y = point[1];
      max.z = point[2];

      is_min_max_initialized = true;
      return;
    }

    for (auto i = 0; i < 3; i++) {
      if (point[i] > max[i]) {
        max[i] = point[i];
      }

      if (point[i] < min[i]) {
        min[i] = point[i];
      }
    }
  };

  if (face_handle.is_valid()) {
    for (auto vertex : mesh_structure.fv_range(face_handle)) {
      a(vertex);
    }
  } else {
    for (auto vertex : mesh_structure.vertices()) {
      a(vertex);
    }
  }

  return AABB(min, max);
}

BVH BVH::from_mesh(MeshStructure& mesh_structure) {
  BVH bvh;

  std::vector<MeshStructure::FaceHandle> faces;
  faces.reserve(mesh_structure.n_faces());

  std::vector<MeshStructure::Point> face_centroid_cache;
  face_centroid_cache.resize(mesh_structure.n_faces());

  std::vector<AABB> face_bounding_boxes_cache;
  face_bounding_boxes_cache.resize(mesh_structure.n_faces());

  for (auto face : mesh_structure.faces()) {
    if (!face.is_valid())
      continue;

    faces.emplace_back(face);
    face_centroid_cache[face.idx()] = mesh_structure.calc_centroid(face);
    face_bounding_boxes_cache[face.idx()] =
        compute_face_bounding_box(mesh_structure, face);
  }

  bvh.root.reset(new BVHNode(true, compute_bounding_box(mesh_structure)));
  bvh.root->primitive_list = std::move(faces);

  std::stack<std::shared_ptr<BVHNode>> stack;
  stack.emplace(bvh.root);

  // choice of bucket count should depend on the density of the mesh. meshes
  // with higher density would require a higher bucket count to increase the
  // chances of spliting to maintain max primitive count.
  const int32_t kBucketCount = 16;
  const int32_t kMaxPrimitiveCount = 16;

  while (!stack.empty()) {
    auto root = stack.top();
    stack.pop();

    if (root->primitive_list.size() < kMaxPrimitiveCount) {
      continue;
    }

    root->is_leaf_node = false;

    float best_split_cost = std::numeric_limits<float>::max();
    float best_split_position = 0;
    uint32_t best_split_axis = 0;
    uint32_t best_split_left_primitive_count = 0;
    uint32_t best_split_right_primitive_count = 0;
    bool split_found = false;

    auto parent_surface_area = root->bounding_box.surface_area();

    for (auto axis = 0; axis < 3; axis++) {
      std::array<Bucket, kBucketCount> buckets;

      auto axisStartPoint = root->bounding_box.bounds[0][axis];
      auto axisEndPoint = root->bounding_box.bounds[1][axis];
      auto step =
          (axisEndPoint - axisStartPoint) / static_cast<float>(kBucketCount);

      for (auto& face : root->primitive_list) {
        auto& centroid = face_centroid_cache[face.idx()];
        int32_t bucketIndex =
            math::clamp((centroid[axis] - axisStartPoint) / step, 0.0f,
                        static_cast<float>(kBucketCount - 1));

        auto& bucket = buckets[bucketIndex];
        bucket.bbox = union_bounding_box(bucket.bbox,
                                         face_bounding_boxes_cache[face.idx()]);
        bucket.primitive_count++;
      }

      for (auto i = 0; i < kBucketCount - 1; i++) {
        // Take split
        // start -> i | i + 1 -> end

        Bucket first{}, second{};

        // for left half

        for (auto j = 0; j < i + 1; j++) {
          first.bbox = union_bounding_box(first.bbox, buckets[j].bbox);
          first.primitive_count += buckets[j].primitive_count;
        }

        if (first.primitive_count == 0)
          continue;

        // for right half

        for (auto j = i + 1; j < kBucketCount; j++) {
          second.bbox = union_bounding_box(second.bbox, buckets[j].bbox);
          second.primitive_count += buckets[j].primitive_count;
        }

        if (second.primitive_count == 0)
          continue;

        float current_split_cost =
            ((first.bbox.surface_area() / parent_surface_area) *
             first.primitive_count) +
            ((second.bbox.surface_area() / parent_surface_area) *
             second.primitive_count);

        if (current_split_cost < best_split_cost) {
          split_found = true;
          best_split_cost = current_split_cost;
          best_split_position = axisStartPoint + (step * (i + 1));
          best_split_axis = axis;
          best_split_left_primitive_count = first.primitive_count;
          best_split_right_primitive_count = second.primitive_count;
        }
      }
    }

    assert(split_found);

    std::shared_ptr<BVHNode> first_child(new BVHNode(true, AABB{})),
        second_child(new BVHNode(true, AABB{}));

    first_child->primitive_list.reserve(best_split_left_primitive_count);
    second_child->primitive_list.reserve(best_split_right_primitive_count);

    for (auto& face : root->primitive_list) {
      auto position_on_axis = face_centroid_cache[face.idx()][best_split_axis];

      if (position_on_axis <= best_split_position) {
        first_child->primitive_list.emplace_back(face);
        first_child->bounding_box = union_bounding_box(
            first_child->bounding_box, face_bounding_boxes_cache[face.idx()]);
      } else {
        second_child->primitive_list.emplace_back(face);
        second_child->bounding_box = union_bounding_box(
            second_child->bounding_box, face_bounding_boxes_cache[face.idx()]);
      }
    }

    root->children[0] = std::move(first_child);
    root->children[1] = std::move(second_child);

    assert(best_split_left_primitive_count ==
           root->children[0]->primitive_list.size());
    assert(best_split_right_primitive_count ==
           root->children[1]->primitive_list.size());

    if (root->children[0]->primitive_list.size() > kMaxPrimitiveCount) {
      stack.emplace(root->children[0]);
    }

    if (root->children[1]->primitive_list.size() > kMaxPrimitiveCount) {
      stack.emplace(root->children[1]);
    }

    // moving the primitive list to ensure it gets deallocated at the end of
    // the current scope
    auto _ = std::move(root->primitive_list);
  }

  return bvh;
}

int32_t BVH::get_tree_height() {
  int32_t height = 0;

  std::queue<std::shared_ptr<BVHNode>> queue;

  if (root) {
    queue.emplace(root);
  }

  while (!queue.empty()) {
    auto nodes_in_current_level = queue.size();

    for (auto i = 0; i < nodes_in_current_level; i++) {
      auto top = queue.front();
      queue.pop();

      if (!top->is_leaf_node) {
        for (auto& child : top->children) {
          queue.emplace(child);
        }
      }
    }

    height++;
  }

  return height;
}

void BVH::intersect(MeshStructure& mesh_structure, math::ray& ray) {
  std::queue<std::shared_ptr<BVHNode>> nodes;

  nodes.emplace(root);

  while (!nodes.empty()) {
    auto current = nodes.front();

    if (!current->bounding_box.intersect_ray(ray).has_value()) {
      nodes.pop();

      continue;
    }
    if (current->is_leaf_node) {
      //
    } else {
      for (auto& child : current->children) {
        nodes.emplace(child);
      }
    }

    nodes.pop();
  }
}
}  // namespace core