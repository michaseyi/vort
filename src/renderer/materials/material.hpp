#pragma once

#include <cstdint>

#include "../shader.hpp"
#include "../uniforms.hpp"
#include "../wgpu_context.hpp"
#include "src/math/math.hpp"

class Material {
public:
    virtual void update(uint32_t tIndex) = 0;

    virtual ResourceGroupEntry resourceGroupEntry() = 0;

    virtual wgpu::ShaderModule shader() = 0;

    virtual uint32_t uniformOffset() = 0;

    virtual void setColor(math::vec3 tColor);
    virtual void setReflectivity(float tReflectivity);
    virtual void setRoughness(float tRoughness);
    virtual void setShininess(float tShininess);

    virtual void setTexture();

    virtual ~Material();
};
