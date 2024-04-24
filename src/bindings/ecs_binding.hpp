#pragma once

#if defined(EMSCRIPTEN)
#include <emscripten/bind.h>
#include <emscripten/emscripten.h>

#include "src/core/render_target_outputs.hpp"
#include "src/ecs/entities.hpp"
#include "src/math/math.hpp"
namespace bindings {
 
class EcsBinding {
 public:
  static std::vector<ecs::EntityId> get_entity_children(
      ecs::EntityId entity_id);

static void print_running_environment();

  static ecs::EntityId create_entity(ecs::EntityId parent_id);

  static void remove_entity(ecs::EntityId entity_id);

  static math::ivec2 get_main_canvas_size();

  static math::ivec2 get_canvas_size(const char* selector);

  static void update_canvas_size(const char* selector);

  // It is assumed the the world would have an address that remains constant
  // for the entire runtime of the app.
  static void set_world(ecs::World& world);

  static ecs::EntityId create_uv_sphere_mesh(ecs::EntityId parent_id);

  static ecs::EntityId create_cube_mesh(ecs::EntityId parent_id);

  static ecs::EntityId create_plane_mesh(ecs::EntityId parent_id);

  static ecs::EntityId create_cylinder_mesh(ecs::EntityId parent_id);

  static ecs::EntityId create_cone_mesh(ecs::EntityId parent_id);

  static ecs::EntityId create_torus_mesh(ecs::EntityId parent_id);

  static void shade_mesh_smooth(ecs::EntityId entity_id);

  static void shade_mesh_normal(ecs::EntityId entity_id);

  static uintptr_t get_entity_position(ecs::EntityId entity_id);

  static uintptr_t get_entity_name(ecs::EntityId entity_id);

  static uintptr_t get_entity_scale(ecs::EntityId entity_id);

  static uint32_t get_entity_interface(ecs::EntityId entity_id);

  static uintptr_t get_entity_orientation(ecs::EntityId entity_id);

  static uintptr_t get_engine_state();

  static void create_render_output(uintptr_t canvas_selelector);

  static void delete_render_output(uintptr_t canvas_selector);

  static void update_render_output(uintptr_t canvas_selector);

  static renderer::HtmlCanvasRenderTarget* find_html_canvas_render_target(
      uintptr_t canvas_selector);

  static void move_editor_camera(uintptr_t canvas_selector, float x_change,
                                 float y_change, float z_change);
  static void rotate_editor_camera(uintptr_t canvas_selector, float pitch_angle,
                                   float yaw_angle);
  static uintptr_t get_rotation_gizmo(uintptr_t canvas_selector);

 private:
  inline static ecs::World* world_;
};
}  // namespace bindings

#endif