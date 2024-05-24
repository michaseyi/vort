#if defined(EMSCRIPTEN)
#include "ecs_binding.hpp"

#include <cstdint>

#include "src/core/camera_manager.hpp"
#include "src/core/components.hpp"
#include "src/renderer/materials/basic_material.hpp"

namespace bindings {

EM_JS(void, _print_running_environment, (),
      { console.log(typeof importScripts == 'undefined' ? "Web" : "Worker"); })

EM_JS(void, _get_canvas_size,
      (int32_t * width, int32_t* height, const char* selector), {
        const canvas = document.querySelector(UTF8ToString(selector));
        const canvas_width = new Int32Array([canvas._width]);
        const canvas_height = new Int32Array([canvas._height]);

        HEAP32.set(canvas_width, width / 4 | 0);
        HEAP32.set(canvas_height, height / 4 | 0);
      });

EM_JS(void, _update_canvas_size, (const char* selector), {
  const canvas = document.querySelector(UTF8ToString(selector));

  canvas.width = canvas._width | 0;
  canvas.height = canvas._height | 0;

  canvas.style.width = `${(canvas.width / window.devicePixelRatio) | 0} px`;
  canvas.style.height = `${(canvas.height / window.devicePixelRatio) | 0} px`;
});

void EcsBinding::print_running_environment() {
  _print_running_environment();
}
void EcsBinding::update_canvas_size(const char* selector) {
  _update_canvas_size(selector);
}
std::vector<ecs::EntityId> EcsBinding::get_entity_children(
    ecs::EntityId entity_id) {
  return world_->get_children(entity_id);
}

ecs::EntityId EcsBinding::create_entity(ecs::EntityId parent_id) {
  return world_->create_entity("Entity0", ecs::EntityInterface::None,
                               parent_id);
}

void EcsBinding::remove_entity(ecs::EntityId entity_id) {
  world_->remove_entity(entity_id);
}

math::ivec2 EcsBinding::get_canvas_size(const char* selector) {
  math::ivec2 canvas_size;
  _get_canvas_size(&canvas_size.x, &canvas_size.y, selector);
  return canvas_size;
}

math::ivec2 EcsBinding::get_main_canvas_size() {
  return get_canvas_size("canvas.emscripten");
}

/**
   * @brief It is assumed that the world would have an address that remains
   * constant for the entire runtime of the app.
   *
   */
void EcsBinding::set_world(ecs::World& world) {
  world_ = &world;
}

ecs::EntityId EcsBinding::create_cube_mesh(ecs::EntityId parent_id) {
  auto new_entity_id =
      world_->create_entity("Cube0", ecs::EntityInterface::Mesh, parent_id);

  world_->set_components(new_entity_id, core::Position(0.0f), core::Scale(1.0f),
                         core::Orientation(),
                         core::Mesh(new core::geometry::Cube(2.0f),
                                    new renderer::materials::BasicMaterial(
                                        renderer::materials::BasicMaterialProps{
                                            .color = {0.5f, 0.5f, 0.5f}})));

  return new_entity_id;
}
ecs::EntityId EcsBinding::create_cylinder_mesh(ecs::EntityId parent_id) {
  auto new_entity_id =
      world_->create_entity("Cylinder0", ecs::EntityInterface::Mesh, parent_id);

  world_->set_components(
      new_entity_id, core::Position(0.0f), core::Scale(1.0f),
      core::Orientation(),
      core::Mesh(new core::geometry::Cylinder(1.0f, 1.0f, 30),
                 new renderer::materials::BasicMaterial(
                     renderer::materials::BasicMaterialProps{
                         .color = {0.5f, 0.5f, 0.5f}})));
  return new_entity_id;
}

ecs::EntityId EcsBinding::create_cone_mesh(ecs::EntityId parent_id) {
  auto new_entity_id =
      world_->create_entity("Cone0", ecs::EntityInterface::Mesh, parent_id);

  world_->set_components(
      new_entity_id, core::Position(0.0f), core::Scale(1.0f),
      core::Orientation(),
      core::Mesh(new core::geometry::Cone(1.0f, 3.0f, 30, 0.0f),
                 new renderer::materials::BasicMaterial(
                     renderer::materials::BasicMaterialProps{
                         .color = {0.5f, 0.5f, 0.5f}})));
  return new_entity_id;
}

ecs::EntityId EcsBinding::create_torus_mesh(ecs::EntityId parent_id) {
  auto new_entity_id =
      world_->create_entity("Torus0", ecs::EntityInterface::Mesh, parent_id);
  world_->set_components(
      new_entity_id, core::Position(0.0f), core::Scale(1.0f),
      core::Orientation(),
      core::Mesh(new core::geometry::Torus(2.0f, 1.5f, 48, 64),
                 new renderer::materials::BasicMaterial(
                     renderer::materials::BasicMaterialProps{
                         .color = {0.5f, 0.5f, 0.5f}})));

  return new_entity_id;
}

ecs::EntityId EcsBinding::create_uv_sphere_mesh(ecs::EntityId parent_id) {
  auto new_entity_id =
      world_->create_entity("UVSphere0", ecs::EntityInterface::Mesh, parent_id);

  world_->set_components(new_entity_id, core::Position(0.0f), core::Scale(1.0f),
                         core::Orientation(),
                         core::Mesh(new core::geometry::UvSphere(40, 20, 1.0f),
                                    new renderer::materials::BasicMaterial(
                                        renderer::materials::BasicMaterialProps{
                                            .color = {0.5f, 0.5f, 0.5f}})));
  return new_entity_id;
}

ecs::EntityId EcsBinding::create_plane_mesh(ecs::EntityId parent_id) {
  auto new_entity_id =
      world_->create_entity("Plane0", ecs::EntityInterface::Mesh, parent_id);

  world_->set_components(new_entity_id, core::Position(0.0f, -10.0f, 0.0f),
                         core::Scale(1.0f), core::Orientation(),
                         core::Mesh(new core::geometry::Plane(6.0f, 6.0f),
                                    new renderer::materials::BasicMaterial(
                                        renderer::materials::BasicMaterialProps{
                                            .color = {0.5f, 0.5f, 0.5f}})));
  return new_entity_id;
}

void EcsBinding::shade_mesh_normal(ecs::EntityId entity_id) {
  auto [mesh] = world_->get_components<core::Mesh>(entity_id);
  mesh.shade_normal();
}

void EcsBinding::shade_mesh_smooth(ecs::EntityId entity_id) {
  auto [mesh] = world_->get_components<core::Mesh>(entity_id);
  mesh.shade_smooth();
}

uint32_t EcsBinding::get_entity_interface(ecs::EntityId entity_id) {
  return static_cast<uint32_t>(world_->get_interface(entity_id));
}

uintptr_t EcsBinding::get_entity_position(ecs::EntityId entity_id) {
  auto [position] = world_->get_components<core::Position>(entity_id);
  return reinterpret_cast<uintptr_t>(&position);
}

uintptr_t EcsBinding::get_entity_scale(ecs::EntityId entity_id) {
  auto [scale] = world_->get_components<core::Scale>(entity_id);
  return reinterpret_cast<uintptr_t>(&scale);
}
uintptr_t EcsBinding::get_entity_orientation(ecs::EntityId entity_id) {
  auto [orientation] = world_->get_components<core::Orientation>(entity_id);
  return reinterpret_cast<uintptr_t>(&orientation);
}

uintptr_t EcsBinding::get_entity_name(ecs::EntityId entity_id) {
  auto& name = world_->get_name(entity_id);

  return reinterpret_cast<uintptr_t>(name.c_str());
}

uintptr_t EcsBinding::get_engine_state() {
  auto [app_state] = world_->get_global<ecs::AppState>();
  return reinterpret_cast<uintptr_t>(&app_state);
}

void EcsBinding::move_editor_camera(uintptr_t canvas_selector, float x_change,
                                    float y_change, float z_change) {
  auto target = find_html_canvas_render_target(canvas_selector);

  assert(target);
  target->camera.move(x_change, y_change, z_change);
}

uintptr_t EcsBinding::get_rotation_gizmo(uintptr_t canvas_selector) {
  auto target = find_html_canvas_render_target(canvas_selector);

  assert(target);
  return reinterpret_cast<uintptr_t>(&target->camera.rotation_gizmo);
}

void EcsBinding::rotate_editor_camera(uintptr_t canvas_selector,
                                      float pitch_angle, float yaw_angle) {
  auto target = find_html_canvas_render_target(canvas_selector);

  assert(target);
  target->camera.rotate(pitch_angle, yaw_angle);
}

void EcsBinding::create_render_output(uintptr_t canvas_selector) {
  auto [render_target_outputs] =
      world_->get_global<core::RenderTargetOutputs>();

  auto target = find_html_canvas_render_target(canvas_selector);
  assert(!target);

  render_target_outputs.outputs.emplace_back(
      renderer::RenderPass{}, renderer::HtmlCanvasRenderTarget(
                                  reinterpret_cast<char*>(canvas_selector)));
}

renderer::HtmlCanvasRenderTarget* EcsBinding::find_html_canvas_render_target(
    uintptr_t canvas_selector) {
  auto [render_target_outputs] =
      world_->get_global<core::RenderTargetOutputs>();

  for (auto& render_target_output : render_target_outputs.outputs) {
    if (render_target_output.html_render_target.canvas_selector ==
        reinterpret_cast<char*>(canvas_selector)) {
      return &render_target_output.html_render_target;
    }
  }
  return nullptr;
}

void EcsBinding::delete_render_output(uintptr_t canvas_selector) {
  auto [render_target_outputs] =
      world_->get_global<core::RenderTargetOutputs>();

  for (auto i = render_target_outputs.outputs.begin();
       i != render_target_outputs.outputs.end(); i++) {
    if (i->html_render_target.canvas_selector ==
        reinterpret_cast<char*>(canvas_selector)) {
      render_target_outputs.outputs.erase(i);
      return;
    }
  }

  assert(false && "You tried to delete a render output that doesnt exist");
}

void EcsBinding::update_render_output(uintptr_t canvas_selector) {
  auto target = find_html_canvas_render_target(canvas_selector);

  assert(target);
  target->output_size =
      EcsBinding::get_canvas_size(reinterpret_cast<char*>(canvas_selector));

  target->update_camera();

  target->update_on_next_render = true;
}

EMSCRIPTEN_BINDINGS(Vort) {
  emscripten::function("globalGetAppState", EcsBinding::get_engine_state);
  emscripten::function("print_running_environment",
                       EcsBinding::print_running_environment);

  emscripten::function("entitiesRemoveEntity", EcsBinding::remove_entity);
  emscripten::function("entitiesCreateEntity", EcsBinding::create_entity);
  emscripten::function("entitiesCreateCubeMesh", EcsBinding::create_cube_mesh);
  emscripten::function("entitiesCreateConeMesh", EcsBinding::create_cone_mesh);
  emscripten::function("entitiesCreateCylinderMesh",
                       EcsBinding::create_cylinder_mesh);
  emscripten::function("entitiesCreatePlaneMesh",
                       EcsBinding::create_plane_mesh);
  emscripten::function("entitiesCreateUVSphereMesh",
                       EcsBinding::create_uv_sphere_mesh);
  emscripten::function("entitiesCreateTorusMesh",
                       EcsBinding::create_torus_mesh);

  emscripten::function("entityGetPosition", EcsBinding::get_entity_position);
  emscripten::function("entityGetChildren", EcsBinding::get_entity_children);
  emscripten::function("entityGetOrientation",
                       EcsBinding::get_entity_orientation);
  emscripten::function("entityGetScale", EcsBinding::get_entity_scale);
  emscripten::function("entityGetInterface", EcsBinding::get_entity_interface);
  emscripten::function("entityGetName", EcsBinding::get_entity_name);

  emscripten::function("meshShadeSmooth", EcsBinding::shade_mesh_smooth);
  emscripten::function("meshShadeNormal", EcsBinding::shade_mesh_normal);

  emscripten::function("editorCreateRenderOutput",
                       EcsBinding::create_render_output);
  emscripten::function("editorDeleteRenderOutput",
                       EcsBinding::delete_render_output);

  emscripten::function("editorUpdateRenderOutput",
                       EcsBinding::update_render_output);
  emscripten::function("editorMoveCamera", EcsBinding::move_editor_camera);
  emscripten::function("editorRotateCamera", EcsBinding::rotate_editor_camera);

  emscripten::function("editorGetRotationGizmoData",
                       EcsBinding::get_rotation_gizmo);
  emscripten::register_vector<ecs::EntityId>("vector<EntityId>");
}
}  // namespace bindings

#endif