#pragma once
#include <limits>
#include <sstream>

#include "cpu_mesh.hpp"
#include "src/math/math.hpp"
#include "src/utils/utils.hpp"

class Geometry {
public:
    virtual void operator()(CPUMesh &) = 0;
    virtual ~Geometry() = default;
};

class ObjLoader : public Geometry {
public:
    ObjLoader(std::string tObjPath) : mObjPath(tObjPath){};

    void operator()(CPUMesh &tMesh) override {
        std::stringstream ss(loadString(mObjPath));
        std::string lineBuffer;

        std::vector<CPUMesh::VertexHandle> faceVertices;
        faceVertices.reserve(3);
        while (!ss.eof()) {
            std::getline(ss, lineBuffer);
            if (lineBuffer.starts_with("v")) {
                std::stringstream line(lineBuffer);
                CPUMesh::Point point;
                line.ignore(2);
                line >> point[0] >> point[1] >> point[2];
                tMesh.add_vertex(point);
            } else if (lineBuffer.starts_with("f")) {
                std::stringstream line(lineBuffer);
                line.ignore(2);
                int32_t vertexIdx;
                line >> vertexIdx;
                faceVertices.emplace_back(vertexIdx - 1);

                line.ignore(std::numeric_limits<std::streamsize>::max(), ' ');
                line >> vertexIdx;
                faceVertices.emplace_back(vertexIdx - 1);

                line.ignore(std::numeric_limits<std::streamsize>::max(), ' ');
                line >> vertexIdx;
                faceVertices.emplace_back(vertexIdx - 1);

                tMesh.add_face(faceVertices);
                faceVertices.clear();
            }
        }
    }

private:
    std::string mObjPath;
};

class Cube : public Geometry {
public:
    Cube(float tSize) : mSize(tSize) {
    }

    void operator()(CPUMesh &tMesh) override {
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

class Plane : public Geometry {
public:
    Plane(float tWidth, float tHeight) : mWidth(tWidth), mHeight(tHeight) {
    }

    void operator()(CPUMesh &tMesh) override {
        float halfWidth = mWidth / 2.0f;
        float halfHeight = mHeight / 2.0f;

        std::vector<CPUMesh::VertexHandle> vHandles = {
            tMesh.add_vertex(CPUMesh::Point(-halfWidth, 0.0f, halfHeight)),
            tMesh.add_vertex(CPUMesh::Point(halfWidth, 0.0f, halfHeight)),
            tMesh.add_vertex(CPUMesh::Point(halfWidth, 0.0f, -halfHeight)),
            tMesh.add_vertex(CPUMesh::Point(-halfWidth, 0.0f, -halfHeight)),
        };

        tMesh.add_face({vHandles[0], vHandles[1], vHandles[3]});
        tMesh.add_face({vHandles[1], vHandles[2], vHandles[3]});
    }

private:
    float mWidth;
    float mHeight;
};

class UVSphere : public Geometry {
public:
    UVSphere(uint32_t tLongitudeSegments, uint32_t tLatitudeSegments, float tRadius)
        : mLongitudeSegments(tLongitudeSegments), mLatitudeSegments(tLatitudeSegments), mRadius(tRadius){};

    void operator()(CPUMesh &tMesh) override {
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

            CPUMesh::Point(mRadius * math::cos(math::radians(90.0f)), mRadius * math::sin(math::radians(90.0f)), 0.0f));
        auto bottomCenter = tMesh.add_vertex(
            CPUMesh::Point(mRadius * math::cos(math::radians(90.0f)), mRadius * math::sin(math::radians(-90.0f)), 0.0f));

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

class Cone : public Geometry {
public:
    Cone(float tRadius, float tHeight, uint32_t tSegments) : mRadius(tRadius), mHeight(tHeight), mSegments(tSegments) {
    }

    void operator()(CPUMesh &tMesh) override {
        auto coneTop = tMesh.add_vertex(CPUMesh::Point(0.0f, mHeight, 0.0f));

        auto coneBottom = tMesh.add_vertex(CPUMesh::Point(0.0f));

        auto twoPi = math::two_pi<float>();

        std::vector<CPUMesh::VertexHandle> circleVhandles;

        for (uint32_t i = 0; i < mSegments; i++) {
            float t = i * (twoPi / mSegments);
            float x = math::cos(t);
            float z = math::sin(t);

            circleVhandles.push_back(tMesh.add_vertex(CPUMesh::Point(x, 0.0f, z)));
        }

        std::vector<CPUMesh::VertexHandle> faceVhandles;
        faceVhandles.reserve(3);
        for (uint32_t i = 0; i < mSegments; i++) {
            faceVhandles.push_back(coneBottom);
            faceVhandles.push_back(circleVhandles[i]);
            faceVhandles.push_back(circleVhandles[(i + 1) % mSegments]);

            tMesh.add_face(faceVhandles);
            faceVhandles.clear();

            faceVhandles.push_back(circleVhandles[i]);
            faceVhandles.push_back(coneTop);
            faceVhandles.push_back(circleVhandles[(i + 1) % mSegments]);
            tMesh.add_face(faceVhandles);
            faceVhandles.clear();
        }
    }

private:
    float mRadius;
    float mHeight;
    uint32_t mSegments;
};

class Cylinder : public Geometry {
public:
    Cylinder(float tRadius, float tHeight, uint32_t tSegments) : mRadius(tRadius), mHeight(tHeight), mSegments(tSegments) {
    }

    void operator()(CPUMesh &tMesh) override {
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
            tMesh.add_face(bottomVhandles[(i + 1) % mSegments], topVhandles[(i + 1) % mSegments], topVhandles[i]);
        }
    }

private:
    float mRadius;
    float mHeight;
    uint32_t mSegments;
};
