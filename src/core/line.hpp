#pragma once
#include <memory>

#include "cpu_mesh.hpp"
#include "geometry.hpp"
#include "src/math/math.hpp"
#include "src/renderer/materials/material.hpp"
#include "src/renderer/wgpu_mesh.hpp"

struct LineVertexData {
    math::vec3 position;
    math::vec3 color;
};

class Mesh {
public:
    Mesh() = default;

    Mesh(Geometry* tGeometry, Material* tMaterial, uint32_t tShaderIndex = 0)
        : geometry(tGeometry), material(tMaterial), mActiveShaderIndex(tShaderIndex) {
        initializeShader();

        mCPUMesh.request_vertex_colors();
        // inline with the arrangment in VertexData Struct
        // position
        mGPUMesh.setAttribute(VertexFormat::Float32x3, offsetof(LineVertexData, position), 0);
        // color
        mGPUMesh.setAttribute(VertexFormat::Float32x3, offsetof(LineVertexData, color), 1);

        (*tGeometry)(mCPUMesh);

        mCPUMesh.update_normals();

        updateGPUMesh();
    }

    void applyTransformation(math::mat4 tTransformationMatrix) {
        for (auto vertex : mCPUMesh.vertices()) {
            auto& point = mCPUMesh.point(vertex);
            auto newPoint = tTransformationMatrix * math::vec4(point[0], point[1], point[2], 1.0f);
            point[0] = newPoint.x;
            point[1] = newPoint.y;
            point[2] = newPoint.z;
        }

        mDirty = true;
    }

    void updateGPUMesh() {
        if (mDirty) {
            std::vector<LineVertexData> data;

            data.reserve(mCPUMesh.n_faces() * 3);

            for (auto edge : mCPUMesh.edges()) {
                for (auto vertex : edge.vertices()) {
                    auto point = mCPUMesh.point(vertex);
                    auto color = mCPUMesh.color(vertex);

                    data.push_back(
                        LineVertexData{.position = {point[0], point[1], point[2]}, .color = {color[0], color[1], color[2]}});
                }
            }
            mGPUMesh.allocateVertexBuffer(data.size() * sizeof(LineVertexData));
            mGPUMesh.writeVertexBuffer(data.data(), data.size() * sizeof(LineVertexData));

            mDirty = false;
        }
    }

    MeshDrawData drawData() {
        MeshDrawData data;
        data.vertexBuffers = std::vector{*mGPUMesh.mVertexBuffer};
        data.indexBuffer = *mGPUMesh.mIndexBuffer;
        data.vertexBufferLayouts = std::vector{mGPUMesh.description()};
        data.vertexCount = mCPUMesh.n_edges() * 2;
        data.vertexShader = mShaders[mActiveShaderIndex].shaderModule;
        data.vertexShaderEntry = "vs_main";
        return data;
    }

public:
    std::unique_ptr<Material> material;
    std::unique_ptr<Geometry> geometry;
    CPUMesh mCPUMesh;
    WGPUMesh mGPUMesh;

private:
    inline static std::vector<Shader> mShaders;
    inline static bool mInitialized = false;
    bool mDirty = true;
    bool mValid = true;
    bool mShadedSmooth = false;
    uint32_t mActiveShaderIndex;
    void initializeShader() {
        if (!mInitialized) {
            mShaders.emplace_back("mesh.wgsl");
            mShaders.emplace_back("screen_space_mesh.wgsl");
            mInitialized = true;
        }
    }
};
