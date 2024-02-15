#include "plain_material.hpp"

#include "../uniforms.hpp"

PlainMaterial::PlainMaterial(PlainMaterialProps tProps) {
    if (!mInitialized) {
        plainMaterialUniform.reset(new Uniform{sizeof(PlainMaterialUniform), true});
        plainMaterialShader.reset(new Shader("plain_material.wgsl"));
        mInitialized = true;
    }
    mResourceGroup.setBindingCount(1);
    mResourceGroup.bindResource(0, plainMaterialUniform);
    mProps = tProps;
    plainMaterialUniform->increment();
}

wgpu::ShaderModule PlainMaterial::shader() {
    return *plainMaterialShader->shaderModule;
}

uint32_t PlainMaterial::uniformOffset() {
    return mCurrentIndex * plainMaterialUniform->stride();
}

ResourceGroupEntry PlainMaterial::resourceGroupEntry() {
    return mResourceGroup.resourceGroupEntry();
}
void PlainMaterial::setColor(math::vec3 tColor) {
    mProps.color = tColor;
}

void PlainMaterial::update(uint32_t tIndex) {
    if (mIncrementOnNext) {
        mCurrentIndex++;
    }
    if (tIndex < mLastSetIndex) {
        mCurrentIndex = 0;
    }
    mLastSetIndex = tIndex + 1;
    mIncrementOnNext = true;

    PlainMaterialUniform uniform{};
    uniform.color = math::vec4(mProps.color, 1.0f);
    plainMaterialUniform->set(mCurrentIndex, 0, sizeof(uniform), &uniform);
}

PlainMaterial::~PlainMaterial() {
    plainMaterialUniform->decrement();
}