#include "basic_material.hpp"

#include "../uniforms.hpp"

BasicMaterial::BasicMaterial(BasicMaterialProps tProps) {
    if (!mInitialized) {
        basicMaterialUniform.reset(new Uniform{sizeof(BasicMaterialUniforms), true});
        basicMaterialShader.reset(new Shader("basic_material.wgsl"));
        mInitialized = true;
    }
    mResourceGroup.setBindingCount(1);
    mResourceGroup.bindResource(0, basicMaterialUniform);
    mProps = tProps;
    basicMaterialUniform->increment();
}

wgpu::ShaderModule BasicMaterial::shader() {
    return *basicMaterialShader->shaderModule;
}

uint32_t BasicMaterial::uniformOffset() {
    return mCurrentIndex * basicMaterialUniform->stride();
}

ResourceGroupEntry BasicMaterial::resourceGroupEntry() {
    return mResourceGroup.resourceGroupEntry();
}
void BasicMaterial::setColor(math::vec3 tColor) {
    mProps.color = tColor;
}

void BasicMaterial::update(uint32_t tIndex) {
    if (mIncrementOnNext) {
        mCurrentIndex++;
    }
    if (tIndex < mLastSetIndex) {
        mCurrentIndex = 0;
    }

    mIncrementOnNext = true;

    mLastSetIndex = tIndex + 1;
    BasicMaterialUniforms uniform{};

    uniform.base_color = mProps.color;
    uniform.alpha = 1.0f;
    basicMaterialUniform->set(mCurrentIndex, 0, sizeof(uniform), &uniform);
}

BasicMaterial::~BasicMaterial() {
    basicMaterialUniform->decrement();
}