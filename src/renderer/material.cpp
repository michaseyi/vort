#include "material.hpp"

#include <iostream>

void Material::setTexture() {
    std::cout << "Selected material does not implment setTexture" << std::endl;
}

void Material::setColor(math::vec3) {
    std::cout << "Selected material does not implment setColor" << std::endl;
}
void Material::setReflectivity(float) {
    std::cout << "Selected material does not implment setReflectivity" << std::endl;
}
void Material::setRoughness(float) {
    std::cout << "Selected material does not implment setRoughness" << std::endl;
}
void Material::setShininess(float) {
    std::cout << "Selected material does not implment setShininess" << std::endl;
}

Material::~Material() = default;

void Material::resetIndex() {
    PhysicsMaterial::mCurrentIndex = 0;
}

PhysicsMaterial::PhysicsMaterial(PhysicsMaterialProps tProps) {
    if (!mInitialized) {
        physicsMaterialPropsUniform.reset(new Uniform<PhysicsMaterialProps>{1});
        physicsMaterialShader.reset(new Shader("assets/shaders/physics_based_material.wgsl"));
        mInitialized = true;
    }
    mProps = tProps;
    physicsMaterialPropsUniform->increment();
}

void PhysicsMaterial::setColor(math::vec3 tColor) {
    mProps.color = tColor;
}
void PhysicsMaterial::setReflectivity(float tReflectivity) {
    mProps.reflectivity = tReflectivity;
}
void PhysicsMaterial::setRoughness(float tRoughness) {
    mProps.roughness = tRoughness;
}
void PhysicsMaterial::setShininess(float tShininess) {
    mProps.shininess = tShininess;
}

void PhysicsMaterial::update() {
    physicsMaterialPropsUniform->set(mCurrentIndex, mProps);
    mCurrentIndex++;
}

PhysicsMaterial::~PhysicsMaterial() {
    physicsMaterialPropsUniform->decrement();
}
