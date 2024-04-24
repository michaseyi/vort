#include "material.hpp"

#include <iostream>

namespace renderer::materials {

void Material::set_texture() {
  std::cout << "Selected material does not implment set_texture" << std::endl;
}

void Material::set_color(math::vec3) {
  std::cout << "Selected material does not implment setColor" << std::endl;
}
void Material::set_reflectivity(float) {
  std::cout << "Selected material does not implment setReflectivity"
            << std::endl;
}
void Material::set_roughness(float) {
  std::cout << "Selected material does not implment set_roughness" << std::endl;
}
void Material::set_shininess(float) {
  std::cout << "Selected material does not implment set_shininess" << std::endl;
}

Material::~Material() = default;

}  // namespace renderer::materials