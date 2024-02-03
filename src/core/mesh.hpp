#pragma once
#include <memory>

#include "cpu_mesh.hpp"
#include "geometry.hpp"
#include "src/math/math.hpp"
#include "src/renderer/material.hpp"
#include "src/renderer/wgpu_mesh.hpp"

struct VertexData {
    math::vec3 position;
    math::vec3 normal;
    math::vec2 uv;
};

class Mesh {
public:
    Mesh() = default;

    Mesh(Geometry* tGeometry, Material* tMaterial) : mGeometry(tGeometry), mMaterial(tMaterial) {
        mCPUMesh.request_vertex_normals();
        mCPUMesh.request_vertex_texcoords2D();
        mCPUMesh.request_face_normals();

        // inline with the arrangment in VertexData Struct
        // position
        mGPUMesh.setAttribute(VertexFormat::Float32x3, offsetof(VertexData, position), 0);
        // normal
        mGPUMesh.setAttribute(VertexFormat::Float32x3, offsetof(VertexData, normal), 1);
        // uv
        mGPUMesh.setAttribute(VertexFormat::Float32x2, offsetof(VertexData, uv), 2);

        (*tGeometry)(mCPUMesh);

        mCPUMesh.update_normals();

        updateGPUMesh();
    }

    void updateGPUMesh() {
        if (mDirty) {
            std::vector<VertexData> data;

            data.reserve(mCPUMesh.n_faces() * 3);

            for (auto face : mCPUMesh.faces()) {
                auto normal = mCPUMesh.normal(face);

                for (auto vertex : face.vertices()) {
                    auto point = mCPUMesh.point(vertex);
                    auto texCoord = mCPUMesh.texcoord2D(vertex);

                    data.push_back(VertexData{.position = {point[0], point[1], point[2]},
                                              .normal = {normal[0], normal[1], normal[2]},
                                              .uv = {texCoord[0], texCoord[1]}});
                }
            }
            mGPUMesh.allocateVertexBuffer(data.size() * sizeof(VertexData));
            mGPUMesh.writeVertexBuffer(data.data(), data.size() * sizeof(VertexData));

            mDirty = false;
        }
    }

    MeshDrawData drawData() {
        MeshDrawData data;
        data.vertexBuffers = std::vector{*mGPUMesh.mVertexBuffer};
        data.indexBuffer = *mGPUMesh.mIndexBuffer;
        data.vertexBufferLayouts = std::vector{mGPUMesh.description()};
        data.vertexCount = mCPUMesh.n_faces() * 3;
        return data;
    }

private:
    CPUMesh mCPUMesh;
    std::unique_ptr<Geometry> mGeometry;
    std::unique_ptr<Material> mMaterial;
    WGPUMesh mGPUMesh;
    bool mDirty = true;
};
