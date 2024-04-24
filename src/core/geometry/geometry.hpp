#pragma once

#include <memory>

#include "../mesh_structure.hpp"
namespace core::geometry {

class Geometry {
 public:
  virtual void operator()(MeshStructure&) = 0;
  virtual ~Geometry() = default;
};

class ObjLoader : public Geometry {
 public:
  ObjLoader(std::string obj_path);

  void operator()(MeshStructure& mesh_structure) override;

 private:
  std::string obj_path_;
};

class Cube : public Geometry {
 public:
  Cube(float size);

  void operator()(MeshStructure& mesh_structure) override;

 private:
  float size_;
};

class Plane : public Geometry {
 public:
  Plane(float width, float height);

  void operator()(MeshStructure& mesh_structure) override;

 private:
  float width_;
  float height_;
};

class UvSphere : public Geometry {
 public:
  UvSphere(uint32_t longitude_segments, uint32_t latitude_segments,
           float radius);

  void operator()(MeshStructure& mesh_structure) override;

 private:
  uint32_t longitude_segments_;
  uint32_t latitude_segments_;
  float radius_ = 1.0f;
};

class CombinedGeometry : public Geometry {
 public:
  CombinedGeometry(Geometry* a, Geometry* b);

  void operator()(MeshStructure& mesh_structure) override;

 private:
  std::vector<std::unique_ptr<Geometry>> geometries_;
};

class Cone : public Geometry {
 public:
  Cone(float radius, float height, uint32_t segments, float offset);

  void operator()(MeshStructure& mesh_structure) override;

 private:
  float radius_;
  float height_;
  uint32_t segments_;
  float offset_;
};

class Cylinder : public Geometry {
 public:
  Cylinder(float radius, float height, uint32_t segments,
           float height_start = 0.0f);

  void operator()(MeshStructure& mesh_structure) override;

 private:
  float radius_;
  float height_;
  float height_start_ = 0.0f;
  uint32_t segments_;
};

class Torus : public Geometry {
 public:
  Torus(float major_radius, float minor_radius, uint32_t segments,
        uint32_t rings);
  void operator()(MeshStructure& mesh_structure) override;

 private:
  float major_radius_;
  float minor_radius_;
  uint32_t segments_;
  uint32_t rings_;
};
}  // namespace core::geometry