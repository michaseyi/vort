#pragma once

#include <cstdint>

#include "shader.hpp"
#include "src/math/math.hpp"
#include "uniforms.hpp"

class Material {
public:
    virtual void update() = 0;

    virtual void setColor(math::vec3 tColor);
    virtual void setReflectivity(float tReflectivity);
    virtual void setRoughness(float tRoughness);
    virtual void setShininess(float tShininess);

    virtual void setTexture();

    static void resetIndex();

    virtual ~Material();
};

struct PhysicsMaterialProps {
    math::vec3 color;
    float reflectivity;
    float roughness;
    float shininess;
};

class PhysicsMaterial : public Material {
public:
    friend class Material;

    PhysicsMaterial(PhysicsMaterialProps tProps);

    void update() override;

    void setColor(math::vec3 tColor) override;
    void setReflectivity(float tReflectivity) override;
    void setRoughness(float tRoughness) override;
    void setShininess(float tShininess) override;

    ~PhysicsMaterial();

    inline static std::unique_ptr<Uniform<PhysicsMaterialProps>> physicsMaterialPropsUniform;
    inline static std::unique_ptr<Shader> physicsMaterialShader;

private:
    inline static bool mInitialized;
    inline static uint64_t mCurrentIndex = 0;

    bool mValid;
    PhysicsMaterialProps mProps;
};