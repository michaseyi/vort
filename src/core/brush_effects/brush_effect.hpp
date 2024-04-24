#pragma once
#include "../mesh_structure.hpp"
#include "src/math/math.hpp"

namespace core::brush_effects {
struct Brush {
  float radius;
  math::vec3 direction;
  math::vec3 position;
};

class BrushEffect {
 public:
  virtual void apply(Brush& brush, MeshStructure& mesh_structure) = 0;
};

class DrawBrush : public BrushEffect {
 public:
  void apply(Brush& brush, MeshStructure& mesh_structure) override;
};

class InflateBrush : public BrushEffect {
 public:
  void apply(Brush& brush, MeshStructure& mesh_structure) override;
};
}  // namespace core::brush_effects