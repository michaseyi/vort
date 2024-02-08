#pragma once
#include <memory>

#include "material.hpp"
#include "src/core/data_structs.hpp"
#include "src/math/math.hpp"

struct BasicMaterialProps {
    math::vec3 color;
};

class BasicMaterial : public Material {
public:
    BasicMaterial(BasicMaterialProps tProps);

    void update(uint32_t tIndex) override;

    void setColor(math::vec3 tColor) override;

    ~BasicMaterial();

    uint32_t uniformOffset() override;
    
    wgpu::ShaderModule shader() override;

    ResourceGroupEntry resourceGroupEntry() override;

    inline static std::shared_ptr<Uniform> basicMaterialUniform;
    inline static std::unique_ptr<Shader> basicMaterialShader;

    BasicMaterial(BasicMaterial &&);
    BasicMaterial& operator=(BasicMaterial &&);
private:
    inline static bool mInitialized;
    inline static uint64_t mCurrentIndex = 0;
    ResourceGroup mResourceGroup;

    bool mValid = true;
    BasicMaterialProps mProps;
};