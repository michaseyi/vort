#pragma once
#include "cpu_mesh.hpp"
#include "src/math/math.hpp"

class Geometry {
public:
    virtual void operator()(CPUMesh &) const = 0;
};

class Cube : public Geometry {
public:
    Cube(float tSize) : mSize(tSize) {
    }

    void operator()(CPUMesh &tMesh) const override {
        std::vector<CPUMesh::VertexHandle> vHandles;
        vHandles.reserve(8);
        vHandles.push_back(tMesh.add_vertex(CPUMesh::Point(-1, -1, 1)));
        vHandles.push_back(tMesh.add_vertex(CPUMesh::Point(1, -1, 1)));
        vHandles.push_back(tMesh.add_vertex(CPUMesh::Point(1, 1, 1)));
        vHandles.push_back(tMesh.add_vertex(CPUMesh::Point(-1, 1, 1)));
        vHandles.push_back(tMesh.add_vertex(CPUMesh::Point(-1, -1, -1)));
        vHandles.push_back(tMesh.add_vertex(CPUMesh::Point(1, -1, -1)));
        vHandles.push_back(tMesh.add_vertex(CPUMesh::Point(1, 1, -1)));
        vHandles.push_back(tMesh.add_vertex(CPUMesh::Point(-1, 1, -1)));

        std::vector<CPUMesh::VertexHandle> faceVhandles;

        std::vector<int> faceIndices = {0, 1, 2, 0, 2, 3, 1, 5, 6, 1, 6, 2, 5, 4, 7, 5, 7, 6,
                                        4, 0, 3, 4, 3, 7, 3, 2, 6, 3, 6, 7, 4, 5, 1, 4, 1, 0};
        for (int i = 0; i < 36; i += 3) {
            faceVhandles.clear();
            for (int j = 0; j < 3; ++j) {
                faceVhandles.push_back(vHandles[faceIndices[i + j]]);
            }

            tMesh.add_face(faceVhandles);
        }
    }

private:
    float mSize;
};

class UVSphere : public Geometry {
public:
    UVSphere(uint32_t tLongitudeSegments, uint32_t tLatitudeSegments, float tRadius)
        : mLongitudeSegments(tLongitudeSegments), mLatitudeSegments(tLatitudeSegments), mRadius(tRadius){};

    void operator()(CPUMesh &tMesh) const override {
        const float PI = math::pi<float>();

        std::vector<std::vector<CPUMesh::VertexHandle>> vHandles;
        for (uint32_t i = 0; i < mLongitudeSegments; ++i) {
            vHandles.push_back({});
            float theta = i * 2 * PI / mLongitudeSegments;

            for (uint32_t j = 1; j < mLatitudeSegments; ++j) {
                float phi = (j * PI / mLatitudeSegments) - (PI / 2.0f);
                float x = mRadius * math::cos(phi) * math::cos(-theta);
                float z = mRadius * math::cos(phi) * math::sin(-theta);
                float y = mRadius * math::sin(phi);
                vHandles[i].push_back(tMesh.add_vertex(CPUMesh::Point(x, y, z)));
            }
        }

        auto topCenter = tMesh.add_vertex(

            CPUMesh::Point(mRadius * math::cos(math::radians(90.0f)),
                           mRadius * math::sin(math::radians(90.0f)), 0.0f));
        auto bottomCenter = tMesh.add_vertex(CPUMesh::Point(
            mRadius * math::cos(math::radians(90.0f)), mRadius * math::sin(math::radians(-90.0f)), 0.0f));

        for (uint32_t i = 0; i < mLongitudeSegments; ++i) {
            auto max_index = vHandles[i].size() - 1;
            std::vector<CPUMesh::VertexHandle> faceVhandles;
            faceVhandles.push_back(vHandles[i][max_index]);
            faceVhandles.push_back(vHandles[(i + 1) % mLongitudeSegments][max_index]);
            faceVhandles.push_back(topCenter);
            tMesh.add_face(faceVhandles);

            faceVhandles.clear();
            faceVhandles.push_back(vHandles[i][0]);
            faceVhandles.push_back(bottomCenter);
            faceVhandles.push_back(vHandles[(i + 1) % mLongitudeSegments][0]);
            tMesh.add_face(faceVhandles);
        }

        // Create faces
        for (uint32_t i = 0; i < vHandles.size(); ++i) {
            for (uint32_t j = 0; j < vHandles[0].size() - 1; ++j) {
                uint32_t nextI = (i + 1) % mLongitudeSegments;
                int nextJ = (j + 1);

                std::vector<CPUMesh::VertexHandle> faceVhandles;
                faceVhandles.push_back(vHandles[i][j]);
                faceVhandles.push_back(vHandles[nextI][j]);
                faceVhandles.push_back(vHandles[nextI][nextJ]);
                tMesh.add_face(faceVhandles);

                faceVhandles.clear();
                faceVhandles.push_back(vHandles[i][j]);
                faceVhandles.push_back(vHandles[nextI][nextJ]);
                faceVhandles.push_back(vHandles[i][nextJ]);
                tMesh.add_face(faceVhandles);
            }
        }
    }

private:
    uint32_t mLongitudeSegments;
    uint32_t mLatitudeSegments;
    float mRadius = 1.0f;
};

class Cylinder : public Geometry {
public:
    Cylinder(float tRadius, float tHeight, float tSegments)
        : mRadius(tRadius), mHeight(tHeight), mSegments(tSegments) {
    }

    void operator()(CPUMesh &tMesh) const override {
        const float PI = math::pi<float>();

        // Create vertices for the top and bottom circles
        std::vector<CPUMesh::VertexHandle> bottomVhandles, topVhandles;
        for (uint32_t i = 0; i < mSegments; ++i) {
            float t = i * (2 * PI) / mSegments;
            float x = mRadius * math::cos(t);
            float y = mRadius * math::sin(t);

            bottomVhandles.push_back(tMesh.add_vertex(CPUMesh::Point(x, y, -mHeight / 2.0f)));
            topVhandles.push_back(tMesh.add_vertex(CPUMesh::Point(x, y, mHeight / 2.0f)));
        }

        CPUMesh::VertexHandle bottomCenter = tMesh.add_vertex(CPUMesh::Point(0, 0, -mHeight / 2.0f));
        CPUMesh::VertexHandle topCenter = tMesh.add_vertex(CPUMesh::Point(0, 0, mHeight / 2.0f));
        // Create faces for the top and bottom circles
        for (uint32_t i = 0; i < mSegments; ++i) {
            // Bottom face
            tMesh.add_face(bottomVhandles[i], bottomCenter, bottomVhandles[(i + 1) % mSegments]);
            // Top face
            tMesh.add_face(topVhandles[i], topVhandles[(i + 1) % mSegments], topCenter);
        }

        // Create faces for the sides of the cylinder
        for (uint32_t i = 0; i < mSegments; ++i) {
            // First triangle
            tMesh.add_face(bottomVhandles[i], bottomVhandles[(i + 1) % mSegments], topVhandles[i]);

            // Second triangle
            tMesh.add_face(bottomVhandles[(i + 1) % mSegments], topVhandles[(i + 1) % mSegments],
                           topVhandles[i]);
        }
    }

private:
    float mRadius;
    float mHeight;
    uint32_t mSegments;
};
