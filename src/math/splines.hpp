#pragma once
#include <map>
#include <vector>
#include "math.hpp"

namespace math {
constexpr mat3 quadratic_weights(vec3(1, -2, 1), vec3(-2, 2, 0), vec3(1, 0, 0));
constexpr mat4 cubic_weights(vec4(-1, 3, -3, 1), vec4(3, -6, 3, 0),
                             vec4(-3, 3, 0, 0), vec4(1, 0, 0, 0));

template <size_t length>
vec<length, float> make_input(float t) {
  vec<length, float> input;
  for (size_t i = 0; i < length; i++) {
    input[length - i - 1] = pow(t, i);
  }
  return input;
}

enum class spline_type {
  bezier,
  hermit,
};

template <size_t length, typename T, spline_type SplineT>
class spline {};

template <size_t length, typename T>
class spline<length, T, spline_type::bezier> {

 public:
  vec<length, T> sample(float t){};
};

template <size_t length, typename T>
class spline<length, T, spline_type::hermit> {
 public:
  using input_type = vec<length, T>;
  input_type sample(float t){};

  void add_knot(input_type& position, input_type& velocity, float t){};
};

template <spline_type SplineT>
using spline2 = spline<2, float, SplineT>;
using spline3 = spline<3, float, SplineT>;
using spline4 = spline<4, float, SplineT>;

}  // namespace math

// using namespace math;
// i32 main() {
//   f32 t = 0.2;

//   vec3 p0(0, 0, 0);
//   vec3 c0(5, 10, 0);
//   vec3 p1(10, 0, 0);

//   std::cout << mat3(p0, c0, p1) * (quadratic_weights * make_input<3>(t));

//   make_input<3>(t);
//   return 0;
// }