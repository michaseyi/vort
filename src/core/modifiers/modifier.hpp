

#include "../mesh_structure.hpp"
namespace core::modifiers {

class Modifier {
 public:
  virtual void modify(MeshStructure& mesh_structure) = 0;
};

class SubdivisionModifier : public Modifier {
 public:
  enum class Algorithm {
    Simple = 0,
    Loop,
    CatmullClark,
  };

  SubdivisionModifier(int32_t subdivision_level,
                      Algorithm subdivision_algorithm)
      : subdivision_level_(subdivision_level),
        subdivision_algorithm_(subdivision_algorithm) {}
  void modify(MeshStructure& mesh_structure) override;

 private:
  int32_t subdivision_level_ = 1;
  Algorithm subdivision_algorithm_;
};

class BevelModifier : public Modifier {
 public:
  BevelModifier(bool bevel_vertices, uint32_t segment_count, float width);

  void modify(MeshStructure& mesh_structure) override;

 private:
  bool bevel_vertices_;
  uint32_t segment_count_;
  float width_;
};
}  // namespace core::modifiers