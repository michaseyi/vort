#include "src/math/math.hpp"
#include "wgpu_types.hpp"

#define INPUT_ATTRIBUTE(flag, type, format)                   \
  template <>                                                 \
  struct InputAttribute<InputAttributeFlag::flag> {           \
    using Type = type;                                        \
    constexpr static int Format = wgpu::VertexFormat::format; \
  };

enum class InputAttributeFlag : uint8_t {
  VertexPosition = 1,
  VertexColor = 2,
  VertexNormal = 4,
  FaceNormal = 8,
  VertexTexCoord = 16,
  VertexPosition2D = 32,
  VertexNormal2D = 64
};

template <InputAttributeFlag attribute>
struct InputAttribute {};

INPUT_ATTRIBUTE(VertexPosition, math::vec3, Float32x3)
INPUT_ATTRIBUTE(VertexColor, math::vec3, Float32x3)
INPUT_ATTRIBUTE(VertexNormal, math::vec3, Float32x3)
INPUT_ATTRIBUTE(FaceNormal, math::vec3, Float32x3)
INPUT_ATTRIBUTE(VertexTexCoord, math::vec2, Float32x2)
INPUT_ATTRIBUTE(VertexPosition2D, math::vec2, Float32x2)
INPUT_ATTRIBUTE(VertexNormal2D, math::vec2, Float32x2)

std::array<InputAttributeFlag, 7> kInputAttributeList = {
    InputAttributeFlag::VertexPosition, InputAttributeFlag::VertexColor,
    InputAttributeFlag::VertexNormal,   InputAttributeFlag::FaceNormal,
    InputAttributeFlag::VertexTexCoord, InputAttributeFlag::VertexPosition2D,
    InputAttributeFlag::VertexNormal2D,
};
class Lines {
  template <InputAttributeFlag attribute>
  auto get() {}

  template <>
  auto get<InputAttributeFlag::FaceNormal>() {
    return std::vector<InputAttribute<InputAttributeFlag::FaceNormal>::Type>();
  }

  void shout() {
    Lines lines;

    for (auto attribute : kInputAttributeList) {
      if (static_cast<uint8_t>(attribute) & lines.state_ != 0) {

        lines.state_ &= ~static_cast<uint8_t>(attribute);
      }
    }
  }

 private:
  uint8_t state_ = 0;
};

class Points {};

class Triangles {};