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
            Iterator &operator++();

            bool operator==(Iterator &tOther);

            T &operator*();
        };
        Iterator begin();
        Iterator end();
    };

    class VertexHandle {
    public:
        VertexHandle(int64_t tIndex = -1) : mIndex(tIndex){};

        bool operator!=(VertexHandle tOther) {
            return mIndex != tOther.mIndex;
        }
        bool operator==(VertexHandle tOther) {
            return mIndex == tOther.mIndex;
        }
        uint64_t idx() {
            return mIndex;
        }

        bool valid() {
            return mIndex > -1;
        }

    private:
        int64_t mIndex;
    };
    class HalfedgeHandle {
    public:
        HalfedgeHandle(int64_t tIndex = -1) : mIndex(tIndex){};

        bool operator!=(HalfedgeHandle tOther) {
            return mIndex != tOther.mIndex;
        }
        bool operator==(HalfedgeHandle tOther) {
            return mIndex == tOther.mIndex;
        }
        uint64_t idx() {
            return mIndex;
        }

        bool valid() {
            return mIndex > -1;
        }

    private:
        int64_t mIndex;
    };
    struct EdgeHandle {
    public:
        EdgeHandle(int64_t tIndex = -1) : mIndex(tIndex){};

        bool operator!=(EdgeHandle tOther) {
            return mIndex != tOther.mIndex;
        }
        bool operator==(EdgeHandle tOther) {
            return mIndex == tOther.mIndex;
        }

        uint64_t idx() {
            return mIndex;
        }

        bool valid() {
            return mIndex > -1;
        }

    private:
        int64_t mIndex;
    };
    struct FaceHandle {
    public:
        FaceHandle(int64_t tIndex = -1) : mIndex(tIndex){};

        bool operator!=(FaceHandle tOther) {
            return mIndex != tOther.mIndex;
        }
        bool operator==(FaceHandle tOther) {
            return mIndex == tOther.mIndex;
        }
        uint64_t idx() {
            return mIndex;
        }

        bool valid() {
            return mIndex > -1;
        }

    private:
        int64_t mIndex;
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
        VertexHandle addVertex(math::vec3 tPosition) {
            int64_t index = mVertices.size();

            mVertices.emplace_back();
            mVertexPositions.emplace_back(tPosition);

            mVertexNormals.emplace_back(0.0f);
            mVertexUVs.emplace_back(0.0f);

            return VertexHandle(index);
        }

        FaceHandle addFace(std::vector<VertexHandle> vHandles) {
            assert(vHandles.size() > 2);

            // ensure that the vertices make a face if the number of vHandles is greater than 3
            if (vHandles.size() > 3) {
                auto startVertexHandle = vHandles[0];

                for (auto i = 1; i < vHandles.size() - 1; i++) {}
            }

            std::vector<HalfedgeHandle> halfedgeHandles;
            halfedgeHandles.resize(vHandles.size());

            std::vector<HalfedgeHandle> halfedgeTwinHandles;
            halfedgeTwinHandles.resize(vHandles.size());

            FaceHandle faceHandle(mFaces.size());
            mFaces.emplace_back();
            mFaceNormals.emplace_back(0.0f);

            for (auto i = 0; i < halfedgeHandles.size(); i++) {
                halfedgeHandles[i] = HalfedgeHandle(mHalfedges.size());
                mHalfedges.emplace_back();
            }

            for (auto i = 0; i < halfedgeHandles.size(); i++) {
                auto &halfedge = this->halfedge(halfedgeHandles[i]);
                auto &vertex = this->vertex(vHandles[i]);

                if (this->vertex(vHandles[i]).halfedge.valid()) {
                    // check if halfedge already exists, warn and ignore addFace
                    // for (auto halfedge : outgoingHalfEdges(vHandles[i])) {
                    //     if (this->halfedge(this->halfedge(halfedge).next).vertex == vHandles[(i + 1) % vHandles.size()]) {
                    //         // handle error
                    //         return FaceHandle();
                    //     }
                    // }

                    // find twin
                    // the twin would have the start vertex the same as the the end vertex of this halfedge
                    for (auto possibleHalfedgeTwin : incomingHalfedges(vHandles[i])) {
                        if (this->halfedge(possibleHalfedgeTwin).vertex == vHandles[(i + 1) % vHandles.size()]) {
                            halfedgeTwinHandles[i] = possibleHalfedgeTwin;
                            break;
                        }
                    }
                }
            }

            this->face(faceHandle).halfedge = halfedgeHandles[0];

            for (auto i = 0; i < halfedgeHandles.size(); i++) {
                auto &halfedge = this->halfedge(halfedgeHandles[i]);

                auto &vertex = this->vertex(vHandles[i]);
                if (vertex.halfedge.valid() == false) {
                    vertex.halfedge = halfedgeHandles[i];
                }

                halfedge.next = halfedgeHandles[(i + 1) % halfedgeHandles.size()];
                halfedge.face = faceHandle;
                halfedge.vertex = vHandles[i];
                halfedge.twin = halfedgeTwinHandles[i];

                if (halfedgeTwinHandles[i].valid()) {
                    auto &twin = this->halfedge(halfedgeTwinHandles[i]);
                    twin.twin = halfedgeHandles[i];
                    halfedge.edge = twin.edge;
                } else {
                    Edge edge;
                    edge.halfedge = halfedgeHandles[i];
                    auto edgeHandle = EdgeHandle(mEdges.size());
                    mEdges.emplace_back(edge);

                    halfedge.edge = edgeHandle;
                }
            }

            return faceHandle;
        }

        std::vector<EdgeHandle> edges(VertexHandle tVertexHandle);

        std::vector<HalfedgeHandle> incomingHalfedges(VertexHandle tVertexHandle) {
            auto &vertex = this->vertex(tVertexHandle);

            std::vector<HalfedgeHandle> result;

            if (vertex.halfedge.valid() == false) {
                return result;
            }
            auto currentIncomingHalfedgeHandleTwin = vertex.halfedge;
            auto currentIncomingHalfedgeHandle = this->halfedge(currentIncomingHalfedgeHandleTwin).twin;
            auto firstIncomingHalfedge = currentIncomingHalfedgeHandle;
            bool reverse = false;
            do {
                if (currentIncomingHalfedgeHandle.valid() == false) {
                    reverse = true;
                    auto startHalfedgeHandle = vertex.halfedge;
                    auto currentHalfedgeHandle = this->halfedge(startHalfedgeHandle).next;

                    while (currentHalfedgeHandle != startHalfedgeHandle) {
                        currentIncomingHalfedgeHandle = currentHalfedgeHandle;
                        currentHalfedgeHandle = this->halfedge(currentHalfedgeHandle).next;
                    }
                }

                if (reverse) {
                    result.emplace_back(currentIncomingHalfedgeHandle);

                    if (this->halfedge(currentIncomingHalfedgeHandle).twin.valid() == false) {
                        break;
                    }

                    auto startHalfedgeHandle = this->halfedge(currentIncomingHalfedgeHandle).twin;
                    auto currentHalfedgeHandle = this->halfedge(startHalfedgeHandle).next;

                    while (currentHalfedgeHandle != startHalfedgeHandle) {
                        currentIncomingHalfedgeHandle = currentHalfedgeHandle;
                        currentHalfedgeHandle = this->halfedge(currentHalfedgeHandle).next;
                    }

                } else {
                    if (currentIncomingHalfedgeHandle == firstIncomingHalfedge) {
                        break;
                    }
                    result.emplace_back(currentIncomingHalfedgeHandle);
                    currentIncomingHalfedgeHandleTwin = this->halfedge(currentIncomingHalfedgeHandle).next;
                    currentIncomingHalfedgeHandle = this->halfedge(currentIncomingHalfedgeHandleTwin).twin;
                }
            } while (true);

            return result;
        }

        std::vector<HalfedgeHandle> outgoingHalfedges(VertexHandle tVertexHandle);

        std::vector<EdgeHandle> edges(FaceHandle tFaceHandle) {
            std::vector<EdgeHandle> result;

            auto &face = this->face(tFaceHandle);

            auto currentHalfedgeHandle = face.halfedge;

            do {
                auto halfedge = this->halfedge(currentHalfedgeHandle);
                result.emplace_back(halfedge.edge);
                currentHalfedgeHandle = halfedge.next;
            } while (face.halfedge != currentHalfedgeHandle);

            return result;
        }
        std::vector<HalfedgeHandle> halfedges(FaceHandle tFaceHandle) {
            std::vector<HalfedgeHandle> result;

            auto &face = this->face(tFaceHandle);

            auto currentHalfedgeHandle = face.halfedge;

            do {
                result.emplace_back(currentHalfedgeHandle);
                auto halfedge = this->halfedge(currentHalfedgeHandle);
                currentHalfedgeHandle = halfedge.next;
            } while (face.halfedge != currentHalfedgeHandle);

            return result;
        }
        std::vector<VertexHandle> vertices(FaceHandle tFaceHandle) {
            std::vector<VertexHandle> result;

            auto &face = this->face(tFaceHandle);

            auto currentHalfedgeHandle = face.halfedge;

            do {
                auto halfedge = this->halfedge(currentHalfedgeHandle);
                result.emplace_back(halfedge.vertex);
                currentHalfedgeHandle = halfedge.next;
            } while (face.halfedge != currentHalfedgeHandle);

            return result;
        }

        std::vector<VertexHandle> vertices(EdgeHandle tEdgeHandle) {
            std::vector<VertexHandle> result;

            auto &edgeHalfedgeHandle = this->halfedge(this->edge(tEdgeHandle).halfedge);

            result.emplace_back(edgeHalfedgeHandle.vertex);
            result.emplace_back(this->halfedge(edgeHalfedgeHandle.next).vertex);

            return result;
        }

        std::vector<FaceHandle> faces(VertexHandle tVertexHandle) {
            std::vector<FaceHandle> result;

            for (auto halfedgeHandle : this->incomingHalfedges(tVertexHandle)) {
                result.emplace_back(this->halfedge(halfedgeHandle).face);
            }

            return result;
        }
        std::vector<FaceHandle> faces(EdgeHandle tEdgeHandle) {
            std::vector<FaceHandle> result;

            auto &edgeHalfedgeHandle = this->halfedge(this->edge(tEdgeHandle).halfedge);
            result.emplace_back(edgeHalfedgeHandle.face);

            if (edgeHalfedgeHandle.twin.valid()) {
                result.emplace_back(this->halfedge(edgeHalfedgeHandle.twin).face);
            }

            return result;
        }

        std::vector<VertexHandle> vertices() {
            std::vector<VertexHandle> result;
            result.reserve(mVertices.size());

            for (uint64_t i = 0; i < mVertices.size(); i++) {
                result.emplace_back(i);
            }
            return result;
        }

        std::vector<FaceHandle> faces() {
            std::vector<FaceHandle> result;
            result.reserve(mFaces.size());

            for (uint64_t i = 0; i < mFaces.size(); i++) {
                result.emplace_back(i);
            }
            return result;
        }

        std::vector<EdgeHandle> edges() {
            std::vector<EdgeHandle> result;
            result.reserve(mEdges.size());

            for (uint64_t i = 0; i < mEdges.size(); i++) {
                result.emplace_back(i);
            }
            return result;
        }
        // For now it assumes a face will always have three halfedges
        void updateFaceNormals() {
            for (uint64_t i = 0; i < mFaces.size(); i++) {
                FaceHandle faceHandle(i);

                std::vector<math::vec3> faceEdgeDirections;
                faceEdgeDirections.reserve(2);

                for (auto &halfedge : this->halfedges(faceHandle)) {
                    if (faceEdgeDirections.size() == 2) {
                        break;
                    }

                    auto faceEdgeDirection =
                        math::normalize(this->position(this->halfedge(this->halfedge(halfedge).next).vertex) -
                                        this->position(this->halfedge(halfedge).vertex));
                    faceEdgeDirections.emplace_back(faceEdgeDirection);
                }

                mFaceNormals[i] = math::normalize(math::cross(faceEdgeDirections[0], faceEdgeDirections[1]));
            }
        }

        void updateVertexNormals() {
            for (uint64_t i = 0; i < mVertices.size(); i++) {
                VertexHandle vertexHandle(i);

                math::vec3 vertexNormal(0.0f);
                uint64_t faceCount = 0;

                for (auto faceHandle : this->faces(vertexHandle)) {
                    vertexNormal += this->normal(faceHandle);
                    faceCount++;
                }
                std::cout << faceCount << "\n";

                if (faceCount > 0) {
                    vertexNormal /= faceCount;
                    vertexNormal = math::normalize(vertexNormal);
                }
                mVertexNormals[i] = vertexNormal;
            }
        }

        math::vec3 &position(VertexHandle tVertexHandle) {
            assert(tVertexHandle.valid());

            assert(mVertexPositions.size() > tVertexHandle.idx());

            return mVertexPositions[tVertexHandle.idx()];
        }

        Vertex &vertex(VertexHandle tVertexHandle) {
            assert(tVertexHandle.valid());

            assert(mVertices.size() > tVertexHandle.idx());

            return mVertices[tVertexHandle.idx()];
        }

        Face &face(FaceHandle tFaceHandle) {
            assert(tFaceHandle.valid());

            assert(mFaces.size() > tFaceHandle.idx());

            return mFaces[tFaceHandle.idx()];
        }

        Halfedge &halfedge(HalfedgeHandle tHalfedgeHandle) {
            assert(tHalfedgeHandle.valid());

            assert(mHalfedges.size() > tHalfedgeHandle.idx());

            return mHalfedges[tHalfedgeHandle.idx()];
        }

        Edge &edge(EdgeHandle tEdgeHandle) {
            assert(tEdgeHandle.valid());

            assert(mEdges.size() > tEdgeHandle.idx());

            return mEdges[tEdgeHandle.idx()];
        }

        math::vec3 &normal(VertexHandle tVertexHandle) {
            assert(tVertexHandle.valid());

            assert(mVertexNormals.size() > tVertexHandle.idx());

            return mVertexNormals[tVertexHandle.idx()];
        }

        math::vec3 &normal(FaceHandle tFaceHandle) {
            assert(tFaceHandle.valid());

            assert(mFaceNormals.size() > tFaceHandle.idx());

            return mFaceNormals[tFaceHandle.idx()];
        }

        math::vec2 &uv(VertexHandle tVertexHandle) {
            assert(tVertexHandle.valid());

            assert(mVertexUVs.size() > tVertexHandle.idx());

            return mVertexUVs[tVertexHandle.idx()];
        }

        uint64_t vertexCount() {
            return mVertices.size();
        }
        uint64_t faceCount() {
            return mFaces.size();
        }

        uint64_t halfedgeCount() {
            return mHalfedges.size();
        }
        uint64_t edgeCount() {
            return mEdges.size();
        }

    private:
        std::vector<Vertex> mVertices;
        std::vector<Face> mFaces;
        std::vector<Edge> mEdges;
        std::vector<Halfedge> mHalfedges;

        // vertex properties
        std::vector<math::vec3> mVertexPositions;
        std::vector<math::vec3> mVertexNormals;
        std::vector<math::vec2> mVertexUVs;

        // face properties
        std::vector<math::vec3> mFaceNormals;
    };

}  // namespace vort::mesh
