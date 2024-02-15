#pragma once
#include <memory>

#include "material.hpp"
#include "src/core/data_structs.hpp"
#include "src/math/math.hpp"

struct PlainMaterialUniform {
    math::vec4 color;
};

struct PlainMaterialProps {
    math::vec3 color;
};

class PlainMaterial : public Material {
public:
    PlainMaterial(PlainMaterialProps tProps);

    void update(uint32_t tIndex) override;

    void setColor(math::vec3 tColor) override;

    ~PlainMaterial();

    uint32_t uniformOffset() override;

    wgpu::ShaderModule shader() override;

    ResourceGroupEntry resourceGroupEntry() override;

    inline static std::shared_ptr<Uniform> plainMaterialUniform;
    inline static std::unique_ptr<Shader> plainMaterialShader;

    PlainMaterial(PlainMaterial &&);
    PlainMaterial &operator=(PlainMaterial &&);

private:
    inline static bool mInitialized;
    inline static uint64_t mCurrentIndex = 0;
    inline static uint64_t mLastSetIndex = 0;

    inline static bool mIncrementOnNext = false;
    ResourceGroup mResourceGroup;

    bool mValid = true;
    PlainMaterialProps mProps;
};