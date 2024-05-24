#include "editor_plugin.hpp"

#include "../bounding_volume.hpp"
#include "../brush_effects/brush_effect.hpp"
#include "../camera_manager.hpp"
#include "../components.hpp"
#include "../data_structs.hpp"
#include "../geometry/geometry.hpp"
#include "../mesh.hpp"
#include "../modifiers/modifier.hpp"
#include "../render_target_outputs.hpp"
#include "../timer.hpp"
#include "../window_events.hpp"
#include "src/bindings/ecs_binding.hpp"
#include "src/ecs/ecs.hpp"
#include "src/renderer/bind_group.hpp"
#include "src/renderer/dynamic_uniform_buffer.hpp"
#include "src/renderer/materials/basic_material.hpp"
#include "src/renderer/render_pass.hpp"
#include "src/renderer/texture.hpp"
#include "src/renderer/uniform_buffer.hpp"
#include "src/renderer/wgpu_context.hpp"

namespace core::plugins {

void EditorPlugin::render_system(ecs::Command& command,
                                 ecs::Global<RenderTargetOutputs>& global) {
  auto [render_target_outputs] = global;
#if !defined(EMSCRIPTEN)
  renderer::WgpuContext::get().get_device().tick();
#endif

  for (auto& [render_pass, canvas_render_target] :
       render_target_outputs.outputs) {
    if (canvas_render_target.update_on_next_render) {
      canvas_render_target.update();
      canvas_render_target.update_on_next_render = false;
    }
    if (canvas_render_target.render_count == -1) {
      render_pass.render(command.get_world(), canvas_render_target);
    } else if (canvas_render_target.render_count > 0) {
      render_pass.render(command.get_world(), canvas_render_target);
      canvas_render_target.render_count--;
    }
  }
}

void EditorPlugin::operator()(ecs::World& world) {
#if defined(EMSCRIPTEN)
  bindings::EcsBinding::set_world(world);
#endif
  auto init_status = renderer::WgpuContext::init();

  assert(init_status);

  // renderer::ResourceGroup resourceGroupZero{};
  // std::shared_ptr<renderer::Uniform> common_uniforms =
  //     std::make_shared<renderer::Uniform>(sizeof(CommonUniforms), false);
  // std::shared_ptr<renderer::Uniform> model_uniforms =
  //     std::make_shared<renderer::Uniform>(sizeof(ModelUniforms), true);

  // resourceGroupZero.setBindingCount(2);
  // resourceGroupZero.bindResource(0, common_uniforms);
  // resourceGroupZero.bindResource(1, model_uniforms);

  // setting important global variables

  // tWorld.setGlobal(RenderTargetOutputs{},
  //                  renderer::GeneralUniforms{std::move(common_uniforms),
  //                                            std::move(model_uniforms)},
  //                  std::move(resourceGroupZero), RotationGizmo{});

  renderer::General general{};
  general.textures;

  // std::array<std::string, 6> texture_paths = {
  //     "albedo.png",   "ao.png",         "height.png",
  //     "metallic.png", "normal-ogl.png", "roughness.png"};

  // std::string root_path = "assets/textures/pbr/whispy-grass-meadow/";

  // for (auto& path : texture_paths) {
  //   general.textures.emplace_back(root_path + path);
  // }
  general.bind_group = renderer::BindGroup::create(
      "General", renderer::BindGroupEntries::from_sequential({
                     general.common_uniform, general.model_uniform,
                     //  general.sampler,
                     //  general.textures[0],
                     //  general.textures[1],
                     //  general.textures[2],
                     //  general.textures[3],
                     //  general.textures[4],
                     //  general.textures[5],
                 }));
  world.set_global<RenderTargetOutputs, renderer::General>({},
                                                           std::move(general));

  // set mesh component init and de-init handler
  world.set_component_init_handler<Mesh>(
      [](ecs::World& world, ecs::EntityId entity_id) {
        // auto [generalUniforms, resourceGroupZero] =
        //     world.getGlobal<renderer::GeneralUniforms,
        //     renderer::ResourceGroup>();
        // generalUniforms.model_uniforms->increment();
        // resourceGroupZero.bindResource(1, generalUniforms.model_uniforms);

        auto [general] = world.get_global<renderer::General>();
        general.model_uniform.push();

        general.bind_group = renderer::BindGroup::create(
            "Common Resources",
            renderer::BindGroupEntries::from_indexed({
                {0, general.common_uniform}, {1, general.model_uniform},
                // {2, general.sampler},
                // {3, general.textures[0]},
                // {4, general.textures[1]},
                // {5, general.textures[2]},
                // {6, general.textures[3]},
                // {7, general.textures[4]},
                // {8, general.textures[5]},
            }));
      });

  world.set_component_deinit_handler<Mesh>(
      [](ecs::World& world, ecs::EntityId entity_id) {
        // auto [generalUniforms, resourceGroupZero] =
        //     world.getGlobal<renderer::GeneralUniforms,
        //     renderer::ResourceGroup>();
        // generalUniforms.model_uniforms->decrement();
        // resourceGroupZero.bindResource(1, generalUniforms.model_uniforms);

        auto [general] = world.get_global<renderer::General>();
        general.model_uniform.pop();

        general.bind_group = renderer::BindGroup::create(
            "Common Resources",
            renderer::BindGroupEntries::from_indexed({
                {0, general.common_uniform}, {1, general.model_uniform},
                // {2, general.sampler},
                // {3, general.textures[0]},
                // {4, general.textures[1]},
                // {5, general.textures[2]},
                // {6, general.textures[3]},
                // {7, general.textures[4]},
                // {8, general.textures[5]},
            }));
      });

  // scene
  auto scene = world.create_entity("Scene", ecs::EntityInterface::Scene);

  // camera
  CameraManager editor_camera(math::vec3(10.0f, 0.0f, 0.0f),
                              CameraSettings(45.0f, 16.0 / 9.0, 0.1f, 1000.0f));
  world.set_components(
      world.create_entity("Camera", ecs::EntityInterface::Camera, scene),
      Position(editor_camera.position), Scale(1.0f),
      Orientation(editor_camera.orientation), editor_camera.settings,
      editor_camera.view, editor_camera.rotation_gizmo);

  // point light
  world.set_components(
      world.create_entity("Point Light", ecs::EntityInterface::Light, scene),
      PointLight(math::vec3(0.0f, 8.0f, -10.0f), 0.8, math::vec3(1.0f),
                 math::vec3(0.000007f, 0.0014f, 1.0f)));

  // ambient light
  world.set_components(
      world.create_entity("Ambient Light", ecs::EntityInterface::Light, scene),
      AmbientLight(math::vec3(1.0f), 0.3));

  world.set_components(
      world.create_entity("Cube", ecs::EntityInterface::Mesh, scene),
      Position(0.0), Scale(1.0f), Orientation(),
      Mesh(new geometry::Cube(4.0f),
           new renderer::materials::BasicMaterial(
               renderer::materials::BasicMaterialProps{
                   .color = {0.5f, 0.5f, 0.5f}})));

  world.add_systems(ecs::SystemSchedule::Update, render_system);
}
}  // namespace core::plugins