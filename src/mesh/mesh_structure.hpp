#pragma once
#include <cassert>
#include <cstdint>
#include <vector>

#include "src/math/math.hpp"

namespace vort::mesh {

template <typename T>
class Range {
 public:
  class Iterator {
   public:
    Iterator& operator++();

    bool operator==(Iterator& other);

    T& operator*();
  };
  Iterator begin();
  Iterator end();
};

class VertexHandle {
 public:
  VertexHandle(int64_t index = -1) : index_(index){};

  bool operator!=(VertexHandle other) { return index_ != other.index_; }
  bool operator==(VertexHandle other) { return index_ == other.index_; }
  int64_t idx() { return index_; }

  bool valid() { return index_ > -1; }

 private:
  int64_t index_;
};
class HalfedgeHandle {
 public:
  HalfedgeHandle(int64_t index = -1) : index_(index){};

  bool operator!=(HalfedgeHandle other) { return index_ != other.index_; }
  bool operator==(HalfedgeHandle other) { return index_ == other.index_; }
  int64_t idx() { return index_; }

  bool valid() { return index_ > -1; }

 private:
  int64_t index_;
};
struct EdgeHandle {
 public:
  EdgeHandle(int64_t index = -1) : index_(index){};

  bool operator!=(EdgeHandle other) { return index_ != other.index_; }
  bool operator==(EdgeHandle other) { return index_ == other.index_; }

  int64_t idx() { return index_; }

  bool valid() { return index_ > -1; }

 private:
  int64_t index_;
};
struct FaceHandle {
 public:
  FaceHandle(int64_t index = -1) : index_(index){};

  bool operator!=(FaceHandle other) { return index_ != other.index_; }
  bool operator==(FaceHandle other) { return index_ == other.index_; }
  int64_t idx() { return index_; }

  bool valid() { return index_ > -1; }

 private:
  int64_t index_;
};

struct Face {
  HalfedgeHandle halfedge;
};

struct Edge {
  HalfedgeHandle halfedge;
};

struct Halfedge {
  HalfedgeHandle twin;
  HalfedgeHandle next;
  VertexHandle vertex;
  EdgeHandle edge;
  FaceHandle face;
};

struct Vertex {
  HalfedgeHandle halfedge;
};

class MeshStructure {
 public:
  VertexHandle add_vertex(math::vec3 position) {
    int64_t index = vertices_.size();

    vertices_.emplace_back();
    vertex_positions_.emplace_back(position);

    vertex_normals_.emplace_back(0.0f);
    vertex_uvs_.emplace_back(0.0f);

    return VertexHandle(index);
  }

  FaceHandle add_face(std::vector<VertexHandle> vhandles) {
    assert(vhandles.size() > 2);

    // ensure that the vertices make a face if the number of vHandles is greater than 3
    if (vhandles.size() > 3) {
      auto start_vertex_handle = vhandles[0];

      for (auto i = 1; i < vhandles.size() - 1; i++) {}
    }

    std::vector<HalfedgeHandle> halfedge_handles;
    halfedge_handles.resize(vhandles.size());

    std::vector<HalfedgeHandle> halfedge_twin_handles;
    halfedge_twin_handles.resize(vhandles.size());

    FaceHandle face_handle(faces_.size());
    faces_.emplace_back();
    face_normals_.emplace_back(0.0f);

    for (auto i = 0; i < halfedge_handles.size(); i++) {
      halfedge_handles[i] = HalfedgeHandle(halfedes_.size());
      halfedes_.emplace_back();
    }

    for (auto i = 0; i < halfedge_handles.size(); i++) {
      auto& halfedge = this->halfedge(halfedge_handles[i]);
      auto& vertex = this->vertex(vhandles[i]);

      if (this->vertex(vhandles[i]).halfedge.valid()) {
        // check if halfedge already exists, warn and ignore addFace
        // for (auto halfedge : outgoingHalfEdges(vHandles[i])) {
        //     if (this->halfedge(this->halfedge(halfedge).next).vertex == vHandles[(i + 1) % vHandles.size()]) {
        //         // handle error
        //         return FaceHandle();
        //     }
        // }

        // find twin
        // the twin would have the start vertex the same as the the end vertex of this halfedge
        for (auto possivle_halfedge_twin : incoming_halfedges(vhandles[i])) {
          if (this->halfedge(possivle_halfedge_twin).vertex ==
              vhandles[(i + 1) % vhandles.size()]) {
            halfedge_twin_handles[i] = possivle_halfedge_twin;
            break;
          }
        }
      }
    }

    this->face(face_handle).halfedge = halfedge_handles[0];

    for (auto i = 0; i < halfedge_handles.size(); i++) {
      auto& halfedge = this->halfedge(halfedge_handles[i]);

      auto& vertex = this->vertex(vhandles[i]);
      if (vertex.halfedge.valid() == false) {
        vertex.halfedge = halfedge_handles[i];
      }

      halfedge.next = halfedge_handles[(i + 1) % halfedge_handles.size()];
      halfedge.face = face_handle;
      halfedge.vertex = vhandles[i];
      halfedge.twin = halfedge_twin_handles[i];

      if (halfedge_twin_handles[i].valid()) {
        auto& twin = this->halfedge(halfedge_twin_handles[i]);
        twin.twin = halfedge_handles[i];
        halfedge.edge = twin.edge;
      } else {
        Edge edge;
        edge.halfedge = halfedge_handles[i];
        auto edge_handle = EdgeHandle(edges_.size());
        edges_.emplace_back(edge);

        halfedge.edge = edge_handle;
      }
    }

    return face_handle;
  }

  std::vector<EdgeHandle> edges(VertexHandle vertex_handle);

  std::vector<HalfedgeHandle> incoming_halfedges(VertexHandle vertex_handle) {
    std::vector<HalfedgeHandle> result;
    int64_t i = 0;
    for (auto& halfedge : halfedes_) {
      if (halfedge.vertex == vertex_handle) {
        auto current_halfedge_handle = HalfedgeHandle(i);
        auto prev_halfedge_handle = HalfedgeHandle();

        do {
          prev_halfedge_handle = current_halfedge_handle;
          current_halfedge_handle =
              this->halfedge(current_halfedge_handle).next;
        } while (current_halfedge_handle != HalfedgeHandle(i));
        result.push_back(prev_halfedge_handle);
      }
      i++;
    }

    return result;

    // auto &vertex = this->vertex(vertex_handle);

    // std::vector<HalfedgeHandle> result;

    // if (vertex.halfedge.valid() == false) {
    //     return result;
    // }
    // auto currentIncomingHalfedgeHandleTwin = vertex.halfedge;
    // auto currentIncomingHalfedgeHandle = this->halfedge(currentIncomingHalfedgeHandleTwin).twin;
    // auto firstIncomingHalfedge = currentIncomingHalfedgeHandle;
    // bool reverse = false;
    // do {
    //     if (currentIncomingHalfedgeHandle.valid() == false) {
    //         reverse = true;
    //         auto startHalfedgeHandle = vertex.halfedge;
    //         auto current_halfedge_handle = this->halfedge(startHalfedgeHandle).next;

    //         while (current_halfedge_handle != startHalfedgeHandle) {
    //             currentIncomingHalfedgeHandle = current_halfedge_handle;
    //             current_halfedge_handle = this->halfedge(current_halfedge_handle).next;
    //         }
    //     }

    //     if (reverse) {
    //         result.emplace_back(currentIncomingHalfedgeHandle);

    //         if (this->halfedge(currentIncomingHalfedgeHandle).twin.valid() == false) {
    //             break;
    //         }

    //         auto startHalfedgeHandle = this->halfedge(currentIncomingHalfedgeHandle).twin;
    //         auto current_halfedge_handle = this->halfedge(startHalfedgeHandle).next;

    //         while (current_halfedge_handle != startHalfedgeHandle) {
    //             currentIncomingHalfedgeHandle = current_halfedge_handle;
    //             current_halfedge_handle = this->halfedge(current_halfedge_handle).next;
    //         }

    //     } else {
    //         if (currentIncomingHalfedgeHandle == firstIncomingHalfedge) {
    //             break;
    //         }
    //         result.emplace_back(currentIncomingHalfedgeHandle);
    //         currentIncomingHalfedgeHandleTwin = this->halfedge(currentIncomingHalfedgeHandle).next;
    //         currentIncomingHalfedgeHandle = this->halfedge(currentIncomingHalfedgeHandleTwin).twin;
    //     }
    // } while (true);

    // return result;
  }

  std::vector<HalfedgeHandle> outgoing_halfedges(VertexHandle vertex_handle) {
    std::vector<HalfedgeHandle> result;
    int64_t i = 0;
    for (auto& halfedge : halfedes_) {
      if (halfedge.vertex == vertex_handle) {
        result.emplace_back(i);
      }
      i++;
    }

    return result;
  }

  std::vector<EdgeHandle> edges(FaceHandle face_handle) {
    std::vector<EdgeHandle> result;

    auto& face = this->face(face_handle);

    auto current_halfedge_handle = face.halfedge;

    do {
      auto halfedge = this->halfedge(current_halfedge_handle);
      result.emplace_back(halfedge.edge);
      current_halfedge_handle = halfedge.next;
    } while (face.halfedge != current_halfedge_handle);

    return result;
  }
  std::vector<HalfedgeHandle> halfedges(FaceHandle face_handle) {
    std::vector<HalfedgeHandle> result;

    auto& face = this->face(face_handle);

    auto current_halfedge_handle = face.halfedge;

    do {
      result.emplace_back(current_halfedge_handle);
      auto halfedge = this->halfedge(current_halfedge_handle);
      current_halfedge_handle = halfedge.next;
    } while (face.halfedge != current_halfedge_handle);

    return result;
  }
  std::vector<VertexHandle> vertices(FaceHandle face_handle) {
    std::vector<VertexHandle> result;

    auto& face = this->face(face_handle);

    auto current_halfedge_handle = face.halfedge;

    do {
      auto halfedge = this->halfedge(current_halfedge_handle);
      result.emplace_back(halfedge.vertex);
      current_halfedge_handle = halfedge.next;
    } while (face.halfedge != current_halfedge_handle);

    return result;
  }

  std::vector<VertexHandle> vertices(EdgeHandle edge_handle) {
    std::vector<VertexHandle> result;

    auto& edge_halfedge_handle =
        this->halfedge(this->edge(edge_handle).halfedge);

    result.emplace_back(edge_halfedge_handle.vertex);
    result.emplace_back(this->halfedge(edge_halfedge_handle.next).vertex);

    return result;
  }

  std::vector<FaceHandle> faces(VertexHandle vertex_handle) {
    std::vector<FaceHandle> result;

    for (auto halfedge_handle : this->incoming_halfedges(vertex_handle)) {
      result.emplace_back(this->halfedge(halfedge_handle).face);
    }

    return result;
  }
  std::vector<FaceHandle> faces(EdgeHandle edge_handle) {
    std::vector<FaceHandle> result;

    auto& edge_halfedge_handle =
        this->halfedge(this->edge(edge_handle).halfedge);
    result.emplace_back(edge_halfedge_handle.face);

    if (edge_halfedge_handle.twin.valid()) {
      result.emplace_back(this->halfedge(edge_halfedge_handle.twin).face);
    }

    return result;
  }

  std::vector<VertexHandle> vertices() {
    std::vector<VertexHandle> result;
    result.reserve(vertices_.size());

    for (uint64_t i = 0; i < vertices_.size(); i++) {
      result.emplace_back(i);
    }
    return result;
  }

  std::vector<FaceHandle> faces() {
    std::vector<FaceHandle> result;
    result.reserve(faces_.size());

    for (uint64_t i = 0; i < faces_.size(); i++) {
      result.emplace_back(i);
    }
    return result;
  }

  std::vector<EdgeHandle> edges() {
    std::vector<EdgeHandle> result;
    result.reserve(edges_.size());

    for (uint64_t i = 0; i < edges_.size(); i++) {
      result.emplace_back(i);
    }
    return result;
  }
  // For now it assumes a face will always have three halfedges
  void update_face_normals() {
    for (uint64_t i = 0; i < faces_.size(); i++) {
      FaceHandle face_handle(i);

      std::vector<math::vec3> face_edge_directions;
      face_edge_directions.reserve(2);

      for (auto& halfedge : this->halfedges(face_handle)) {
        if (face_edge_directions.size() == 2) {
          break;
        }

        auto face_edge_direction = math::normalize(
            this->position(
                this->halfedge(this->halfedge(halfedge).next).vertex) -
            this->position(this->halfedge(halfedge).vertex));
        face_edge_directions.emplace_back(face_edge_direction);
      }

      face_normals_[i] = math::normalize(
          math::cross(face_edge_directions[0], face_edge_directions[1]));
    }
  }

  void update_vertex_normals() {
    for (uint64_t i = 0; i < vertices_.size(); i++) {
      VertexHandle vertex_handle(i);

      math::vec3 vertex_normal(0.0f);
      uint64_t face_count = 0;

      for (auto face_handle : this->faces(vertex_handle)) {
        vertex_normal += this->normal(face_handle);
        face_count++;
      }
      std::cout << face_count << "\n";

      if (face_count > 0) {
        vertex_normal /= face_count;
        vertex_normal = math::normalize(vertex_normal);
      }
      vertex_normals_[i] = vertex_normal;
    }
  }

  math::vec3& position(VertexHandle vertex_handle) {
    assert(vertex_handle.valid());

    assert(vertex_positions_.size() > vertex_handle.idx());

    return vertex_positions_[vertex_handle.idx()];
  }

  Vertex& vertex(VertexHandle vertex_handle) {
    assert(vertex_handle.valid());

    assert(vertices_.size() > vertex_handle.idx());

    return vertices_[vertex_handle.idx()];
  }

  Face& face(FaceHandle face_handle) {
    assert(face_handle.valid());

    assert(faces_.size() > face_handle.idx());

    return faces_[face_handle.idx()];
  }

  Halfedge& halfedge(HalfedgeHandle halfedge_handle) {
    assert(halfedge_handle.valid());

    assert(halfedes_.size() > halfedge_handle.idx());

    return halfedes_[halfedge_handle.idx()];
  }

  Edge& edge(EdgeHandle edge_handle) {
    assert(edge_handle.valid());

    assert(edges_.size() > edge_handle.idx());

    return edges_[edge_handle.idx()];
  }

  math::vec3& normal(VertexHandle vertex_handle) {
    assert(vertex_handle.valid());

    assert(vertex_normals_.size() > vertex_handle.idx());

    return vertex_normals_[vertex_handle.idx()];
  }

  math::vec3& normal(FaceHandle face_handle) {
    assert(face_handle.valid());

    assert(face_normals_.size() > face_handle.idx());

    return face_normals_[face_handle.idx()];
  }

  math::vec2& uv(VertexHandle vertex_handle) {
    assert(vertex_handle.valid());

    assert(vertex_uvs_.size() > vertex_handle.idx());

    return vertex_uvs_[vertex_handle.idx()];
  }

  uint64_t vertexCount() { return vertices_.size(); }
  uint64_t faceCount() { return faces_.size(); }

  uint64_t halfedgeCount() { return halfedes_.size(); }
  uint64_t edgeCount() { return edges_.size(); }

 private:
  std::vector<Vertex> vertices_;
  std::vector<Face> faces_;
  std::vector<Edge> edges_;
  std::vector<Halfedge> halfedes_;

  // vertex properties
  std::vector<math::vec3> vertex_positions_;
  std::vector<math::vec3> vertex_normals_;
  std::vector<math::vec2> vertex_uvs_;

  // face properties
  std::vector<math::vec3> face_normals_;
};

}  // namespace vort::mesh
