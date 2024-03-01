#include "modifier.hpp"

class Bevel : public Modifier {
public:
    Bevel(bool tBevelVertices, uint32_t tSegmentCount, float tWidth)
        : mBevelVertices(tBevelVertices), mSegmentCount(tSegmentCount), mWidth(tWidth){};
    void modify(CPUMesh &tMesh) override {
        for (auto vertex: tMesh.vertices()) {
        }
    }

private:
    bool mBevelVertices;
    uint32_t mSegmentCount;
    float mWidth;
};