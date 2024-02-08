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
