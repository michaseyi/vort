#pragma once
#include <memory>

#include "cpu_mesh.hpp"
#include "geometry.hpp"
#include "src/math/math.hpp"
#include "src/mesh/mesh_structure.hpp"
#include "src/renderer/materials/material.hpp"
#include "src/renderer/render_mesh_buffers.hpp"

class UVSphereTest {
public:
    UVSphereTest(uint32_t tLongitudeSegments, uint32_t tLatitudeSegments, float tRadius)
        : mLongitudeSegments(tLongitudeSegments), mLatitudeSegments(tLatitudeSegments), mRadius(tRadius){};

    void operator()(vort::mesh::MeshStructure& tMesh) {
        const float PI = math::pi<float>();

        std::vector<std::vector<vort::mesh::VertexHandle>> vHandles;
        for (uint32_t i = 0; i < mLongitudeSegments; ++i) {
            vHandles.push_back({});
            float theta = i * 2 * PI / mLongitudeSegments;

            for (uint32_t j = 1; j < mLatitudeSegments; ++j) {
                float phi = (j * PI / mLatitudeSegments) - (PI / 2.0f);
                float x = mRadius * math::cos(phi) * math::cos(-theta);
                float z = mRadius * math::cos(phi) * math::sin(-theta);
                float y = mRadius * math::sin(phi);
                vHandles[i].push_back(tMesh.addVertex(math::vec3(x, y, z)));
            }
        }

        auto topCenter = tMesh.addVertex(

            math::vec3(mRadius * math::cos(math::radians(90.0f)), mRadius * math::sin(math::radians(90.0f)), 0.0f));
        auto bottomCenter = tMesh.addVertex(
            math::vec3(mRadius * math::cos(math::radians(90.0f)), mRadius * math::sin(math::radians(-90.0f)), 0.0f));

        for (uint32_t i = 0; i < mLongitudeSegments; ++i) {
            auto max_index = vHandles[i].size() - 1;
            std::vector<vort::mesh::VertexHandle> faceVhandles;
            faceVhandles.push_back(vHandles[i][max_index]);
            faceVhandles.push_back(vHandles[(i + 1) % mLongitudeSegments][max_index]);
            faceVhandles.push_back(topCenter);
            tMesh.addFace(faceVhandles);

            faceVhandles.clear();
            faceVhandles.push_back(vHandles[i][0]);
            faceVhandles.push_back(bottomCenter);
            faceVhandles.push_back(vHandles[(i + 1) % mLongitudeSegments][0]);
            tMesh.addFace(faceVhandles);
        }

        // Create faces
        for (uint32_t i = 0; i < vHandles.size(); ++i) {
            for (uint32_t j = 0; j < vHandles[0].size() - 1; ++j) {
                uint32_t nextI = (i + 1) % mLongitudeSegments;
                int nextJ = (j + 1);

                std::vector<vort::mesh::VertexHandle> faceVhandles;
                faceVhandles.push_back(vHandles[i][j]);
                faceVhandles.push_back(vHandles[nextI][j]);
                faceVhandles.push_back(vHandles[nextI][nextJ]);
                tMesh.addFace(faceVhandles);

                faceVhandles.clear();
                faceVhandles.push_back(vHandles[i][j]);
                faceVhandles.push_back(vHandles[nextI][nextJ]);
                faceVhandles.push_back(vHandles[i][nextJ]);
                tMesh.addFace(faceVhandles);
            }
        }
    }

private:
    uint32_t mLongitudeSegments;
    uint32_t mLatitudeSegments;
    float mRadius = 1.0f;
};

class Mesh {
public:
    Mesh() = default;

    Mesh(Geometry* tGeometry, Material* tMaterial) : geometry(tGeometry), material(tMaterial) {
        initializeShader();

        // mCPUMesh.request_vertex_normals();
        // mCPUMesh.request_vertex_texcoords2D();
        // mCPUMesh.request_face_normals();
        // (*tGeometry)(mCPUMesh);
        // mCPUMesh.update_normals();

        UVSphereTest(32, 16, 2.0f)(mMeshStructure);
        mMeshStructure.updateFaceNormals();
        mMeshStructure.updateVertexNormals();

        // mFaceBuffers.newBuffer(VertexFormat::Float32x3, mCPUMesh.n_faces() * 3, 0);  // position
        // mFaceBuffers.newBuffer(VertexFormat::Float32x3, mCPUMesh.n_faces() * 3, 1);  // color
        // mFaceBuffers.newBuffer(VertexFormat::Float32x3, mCPUMesh.n_faces() * 3, 2);  // normal
        // mFaceBuffers.newBuffer(VertexFormat::Float32x2, mCPUMesh.n_faces() * 3, 3);  // uv

        // mLineBuffers.newBuffer(VertexFormat::Float32x3, mCPUMesh.n_edges() * 2, 0);  // position
        // mLineBuffers.newBuffer(VertexFormat::Float32x3, mCPUMesh.n_edges() * 2, 1);  // color

        // mPointBuffers.newBuffer(VertexFormat::Float32x3, mCPUMesh.n_vertices(), 0, true);  // position
        // mPointBuffers.newBuffer(VertexFormat::Float32x3, mCPUMesh.n_vertices(), 1, true);  // color

        mFaceBuffers.newBuffer(VertexFormat::Float32x3, mMeshStructure.faceCount() * 3, 0);  // position
        mFaceBuffers.newBuffer(VertexFormat::Float32x3, mMeshStructure.faceCount() * 3, 1);  // color
        mFaceBuffers.newBuffer(VertexFormat::Float32x3, mMeshStructure.faceCount() * 3, 2);  // normal
        mFaceBuffers.newBuffer(VertexFormat::Float32x2, mMeshStructure.faceCount() * 3, 3);  // uv

        mLineBuffers.newBuffer(VertexFormat::Float32x3, mMeshStructure.edgeCount() * 2, 0);  // position
        mLineBuffers.newBuffer(VertexFormat::Float32x3, mMeshStructure.edgeCount() * 2, 1);  // color

        mPointBuffers.newBuffer(VertexFormat::Float32x3, mMeshStructure.vertexCount(), 0, true);  // position
        mPointBuffers.newBuffer(VertexFormat::Float32x3, mMeshStructure.vertexCount(), 1, true);  // color
        updateGPUMesh();
    }

    void shadeSmooth() {
        if (!mShadedSmooth) {
            mShadedSmooth = true;
            mFaceBuffersNeedUpdate = true;
        }
    }
    void shadeNormal() {
        if (mShadedSmooth) {
            mShadedSmooth = false;
            mFaceBuffersNeedUpdate = true;
        }
    }

    void applyTransformation(math::mat4 tTransformationMatrix) {
        // for (auto vertex : mCPUMesh.vertices()) {
        //     auto& point = mCPUMesh.point(vertex);
        //     auto newPoint = tTransformationMatrix * math::vec4(point[0], point[1], point[2], 1.0f);
        //     point[0] = newPoint.x;
        //     point[1] = newPoint.y;
        //     point[2] = newPoint.z;
        // }

        // mCPUMesh.update_normals();

        for (auto vertex : mMeshStructure.vertices()) {
            auto& position = mMeshStructure.position(vertex);
            position = tTransformationMatrix * math::vec4(position, 1.0f);
        }
        mMeshStructure.updateFaceNormals();
        mMeshStructure.updateVertexNormals();

        mLineBuffersNeedUpdate = true;
        mFaceBuffersNeedUpdate = true;
        mPointBuffersNeedUpdate = true;
    }

    void updateGPUMesh() {
        updateAllFaceBuffers();
        updateAllLineBuffers();
        updateAllPointBuffers();
    }

    MeshDrawData facesDrawData() {
        MeshDrawData data;
        data.vertexBuffers = mFaceBuffers.getBuffers();
        data.vertexBufferLayouts = mFaceBuffers.getBufferLayouts();
        // data.vertexCount = mCPUMesh.n_faces() * 3;
        data.vertexCount = mMeshStructure.faceCount() * 3;
        data.instanceCount = 1;
        data.vertexShader = mShader->shaderModule;
        data.fragmentShader = mShader->shaderModule;
        data.vertexShaderEntry = "vs_main_face";
        data.fragmentShaderEntry = "fs_main_face";
        data.topology = PrimitiveTopology::TriangleList;
        return data;
    }
    MeshDrawData edgesDrawData() {
        MeshDrawData data;
        data.vertexBuffers = mLineBuffers.getBuffers();
        data.vertexBufferLayouts = mLineBuffers.getBufferLayouts();
        // data.vertexCount = mCPUMesh.n_edges() * 2;
        data.vertexCount = mMeshStructure.edgeCount() * 2;
        data.instanceCount = 1;
        data.vertexShader = mShader->shaderModule;
        data.fragmentShader = mShader->shaderModule;
        data.vertexShaderEntry = "vs_main_edge";
        data.fragmentShaderEntry = "fs_main_edge";
        data.topology = PrimitiveTopology::LineList;
        return data;
    }
    MeshDrawData verticesDrawData() {
        MeshDrawData data;
        data.vertexBuffers = mPointBuffers.getBuffers();
        data.vertexBufferLayouts = mPointBuffers.getBufferLayouts();
        data.vertexCount = 6;
        // data.instanceCount = mCPUMesh.n_vertices();
        data.instanceCount = mMeshStructure.vertexCount();
        data.vertexShader = mShader->shaderModule;
        data.fragmentShader = mShader->shaderModule;
        data.vertexShaderEntry = "vs_main_point";
        data.fragmentShaderEntry = "fs_main_point";
        data.topology = PrimitiveTopology::TriangleList;
        return data;
    }

    void updateAllFaceBuffers() {
        if (!mFaceBuffersNeedUpdate) {
            return;
        }
        mFaceBuffersNeedUpdate = false;

        std::vector<math::vec3> positions;
        std::vector<math::vec3> colors;
        std::vector<math::vec3> normals;
        std::vector<math::vec2> uvs;

        // auto size = mCPUMesh.n_faces() * 3;
        // positions.reserve(size);
        // normals.reserve(size);
        // colors.reserve(size);
        // uvs.reserve(size);

        // math::vec3 color{0.5f};
        // for (auto face : mCPUMesh.faces()) {
        //     auto normal = mCPUMesh.normal(face);

        //     for (auto vertex : face.vertices()) {
        //         auto point = mCPUMesh.point(vertex);
        //         auto texCoord = mCPUMesh.texcoord2D(vertex);

        //         if (mShadedSmooth) {
        //             normal = mCPUMesh.normal(vertex);
        //         }
        //         positions.emplace_back(point[0], point[1], point[2]);
        //         normals.emplace_back(normal[0], normal[1], normal[2]);
        //         colors.emplace_back(color[0], color[1], color[2]);
        //         uvs.emplace_back(texCoord[0], texCoord[1]);
        //     }
        // }

        auto size = mMeshStructure.faceCount() * 3;
        positions.reserve(size);
        normals.reserve(size);
        colors.reserve(size);
        uvs.reserve(size);

        math::vec3 color{0.5f};
        for (auto face : mMeshStructure.faces()) {
            auto normal = mMeshStructure.normal(face);

            for (auto vertex : mMeshStructure.vertices(face)) {
                auto position = mMeshStructure.position(vertex);
                auto uv = mMeshStructure.uv(vertex);

                if (mShadedSmooth) {
                    normal = mMeshStructure.normal(vertex);
                    std::cout << normal << "\n";
                }
                positions.emplace_back(position[0], position[1], position[2]);
                normals.emplace_back(normal[0], normal[1], normal[2]);
                colors.emplace_back(color[0], color[1], color[2]);
                uvs.emplace_back(uv[0], uv[1]);
            }
        }

        mFaceBuffers.writeBuffer(0, 0, size, positions.data());
        mFaceBuffers.writeBuffer(1, 0, size, colors.data());
        mFaceBuffers.writeBuffer(2, 0, size, normals.data());
        mFaceBuffers.writeBuffer(3, 0, size, uvs.data());
    }

    void updateAllLineBuffers() {
        if (!mLineBuffersNeedUpdate) {
            return;
        }
        mLineBuffersNeedUpdate = false;

        std::vector<math::vec3> positions;
        std::vector<math::vec3> colors;

        // auto size = mCPUMesh.n_edges() * 2;

        // positions.reserve(size);
        // colors.reserve(size);

        // math::vec3 color{0.0f};
        // for (auto edge : mCPUMesh.edges()) {
        //     for (auto vertex : edge.vertices()) {
        //         auto point = mCPUMesh.point(vertex);

        //         positions.emplace_back(point[0], point[1], point[2]);
        //         colors.emplace_back(color[0], color[1], color[2]);
        //     }
        // }
        auto size = mMeshStructure.edgeCount() * 2;

        positions.reserve(size);
        colors.reserve(size);

        math::vec3 color{0.0f};
        for (auto edge : mMeshStructure.edges()) {
            for (auto vertex : mMeshStructure.vertices(edge)) {
                auto position = mMeshStructure.position(vertex);
                positions.emplace_back(position[0], position[1], position[2]);
                colors.emplace_back(color[0], color[1], color[2]);
            }
        }

        mLineBuffers.writeBuffer(0, 0, size, positions.data());
        mLineBuffers.writeBuffer(1, 0, size, colors.data());
    }

    void updateAllPointBuffers() {
        if (!mPointBuffersNeedUpdate) {
            return;
        }

        mPointBuffersNeedUpdate = false;

        std::vector<math::vec3> positions;
        std::vector<math::vec3> colors;

        // auto size = mCPUMesh.n_vertices();

        // positions.reserve(size);
        // colors.reserve(size);

        // math::vec3 color{0.0f};
        // for (auto vertex : mCPUMesh.vertices()) {
        //     auto point = mCPUMesh.point(vertex);
        //     positions.emplace_back(point[0], point[1], point[2]);
        //     colors.emplace_back(color[0], color[1], color[2]);
        // }

        auto size = mMeshStructure.vertexCount();

        positions.reserve(size);
        colors.reserve(size);

        math::vec3 color{0.0f};
        for (auto vertex : mMeshStructure.vertices()) {
            auto position = mMeshStructure.position(vertex);
            positions.emplace_back(position[0], position[1], position[2]);
            colors.emplace_back(color[0], color[1], color[2]);
        }

        mPointBuffers.writeBuffer(0, 0, size, positions.data());
        mPointBuffers.writeBuffer(1, 0, size, colors.data());
    }

public:
    std::unique_ptr<Material> material;
    std::unique_ptr<Geometry> geometry;

private:
    CPUMesh mCPUMesh;

    RenderMeshBuffers mFaceBuffers;
    RenderMeshBuffers mLineBuffers;
    RenderMeshBuffers mPointBuffers;

    bool mFaceBuffersNeedUpdate = true;
    bool mLineBuffersNeedUpdate = true;
    bool mPointBuffersNeedUpdate = true;

    inline static std::unique_ptr<Shader> mShader;
    inline static bool mInitialized = false;
    bool mValid = true;
    bool mShadedSmooth = true;

    vort::mesh::MeshStructure mMeshStructure;

    void initializeShader() {
        if (!mInitialized) {
            mShader.reset(new Shader("mesh.wgsl"));
            mInitialized = true;
        }
    }
};
