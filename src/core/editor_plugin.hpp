#pragma once
#include "bounding_volume.hpp"
#include "camera_controller.hpp"
#include "components.hpp"
#include "data_structs.hpp"
#include "geometry.hpp"
#include "mesh.hpp"
#include "src/bindings/ecs_binding.hpp"
#include "src/ecs/ecs.hpp"
#include "src/renderer/materials/basic_material.hpp"
#include "src/renderer/materials/plain_material.hpp"
#include "src/renderer/render_pass.hpp"
#include "src/renderer/texture.hpp"
#include "src/renderer/wgpu_context.hpp"
#include "window.hpp"
#include "window_events.hpp"

struct TranslationGizmo {};
struct EditorPlugin {
    static void renderSystem(Command& command, Global<RenderPass>& global) {
        auto [renderPass] = global;

#if !defined(EMSCRIPTEN)
        auto& context = WGPUContext::getContext();
        context.getDevice().tick();
#endif

        renderPass.render(command);
    }

    static void windowUpdate(Global<Window, AppState>& global) {
        auto [window, appState] = global;
        window.processEvents();

        if (!window.isOpen()) {
            appState.running = false;
        }
    }

    static void gizmoInit(Command& command) {
        // auto gizmoQuery = command.query<Columns<Position, Scale, Orientation, Mesh>, Tags<TranslationGizmo>>();

        // for (auto [position, scale, orientation, mesh] : gizmoQuery) {
        //     math::mat4 modelMatrix = math::translate(math::mat4(1.0f), math::vec3(position));
        //     modelMatrix = math::rotate(modelMatrix, math::angle(math::quat(orientation)), math::axis(math::quat(orientation)));
        //     modelMatrix = math::scale(modelMatrix, math::vec3(scale));

        //     mesh.applyTransformation(modelMatrix);

        //     position = Position(0.0f);
        //     scale = Scale(1.0f);
        //     orientation = Orientation();
        // }
    }

    static void gizmoUpdate(Command& command) {
        // auto gizmoQuery = command.query<Columns<Position>, Tags<TranslationGizmo>>();

        // auto [cameraPosition, cameraOrientation, cameraSettings] =
        //     command.query<Columns<Position, Orientation, CameraSettings>, Tags<>>().single();
        // auto cameraFront = math::quat(cameraOrientation) * math::vec3(1.0f, 0.0f, 0.0f);
        // for (auto [position] : gizmoQuery) {
        //     // position = math::vec3(-1.0f, 1.0f, 4.0f);
        // }
    }

    void operator()(World& tWorld) {
        int32_t width = 800, height = 800;
#if defined(EMSCRIPTEN)
        ECSBinding::setWorld(tWorld);

        auto canvasSize = ECSBinding::getMainWindowCanvasSize();
        width = canvasSize.x;
        height = canvasSize.y;
#endif
        auto window = WindowBuilder().clientAPI(ClientAPI::None).size(width, height).build();

        assert(window);

        auto wgpuInitStatus = WGPUContext::init(&window);

        assert(wgpuInitStatus);

        ResourceGroup resourceGroupZero{};
        std::shared_ptr<Uniform> commonUniforms = std::make_shared<Uniform>(sizeof(CommonUniforms), false);
        std::shared_ptr<Uniform> modelUniforms = std::make_shared<Uniform>(sizeof(ModelUniforms), true);

        resourceGroupZero.setBindingCount(2);
        resourceGroupZero.bindResource(0, commonUniforms);
        resourceGroupZero.bindResource(1, modelUniforms);

        tWorld.setGlobal(RenderPass{}, GeneralUniforms{std::move(commonUniforms), std::move(modelUniforms)},
                         std::move(resourceGroupZero), RotationGizmo{});

        // set mesh component init and deint handler
        tWorld.setComponentInitHandler<Mesh>([](World& world, EntityID entityID) {
            auto [generalUniforms, resourceGroupZero] = world.getGlobal<GeneralUniforms, ResourceGroup>();
            generalUniforms.modelUniforms->increment();
            resourceGroupZero.bindResource(1, generalUniforms.modelUniforms);
        });

        tWorld.setComponentDeinitHandler<Mesh>([](World& world, EntityID entityID) {
            auto [generalUniforms, resourceGroupZero] = world.getGlobal<GeneralUniforms, ResourceGroup>();
            generalUniforms.modelUniforms->decrement();
            resourceGroupZero.bindResource(1, generalUniforms.modelUniforms);
        });

        // scene
        auto scene = tWorld.newEntity("Scene", EntityInterface::Scene);

        // camera
        auto cameraEntity = tWorld.newEntity("Camera", EntityInterface::Camera, scene);

        tWorld.setComponents(
            cameraEntity, Position(12.0f, 0.0f, 0.0f), Scale(1.0f), Orientation(),
            CameraSettings(45.0f, static_cast<float>(window.width()) / static_cast<float>(window.height()), 0.1f, 1000.0f),
            View());

        auto editorCameraController = CameraController();

        auto pointLightEntity = tWorld.newEntity("Point Light", EntityInterface::Light, scene);
        tWorld.setComponents(pointLightEntity, PointLight(math::vec3(0.0f, 8.0f, -10.0f), 0.8, math::vec3(1.0f),
                                                          math::vec3(0.000007f, 0.0014f, 1.0f)));

        editorCameraController.attachCamera(cameraEntity)
            .attachLight(pointLightEntity)
            .init(tWorld);
            // .rotate(tWorld, math::radians(-45.0f * 100), math::radians(45.0f * 100));

        tWorld.setGlobal(editorCameraController);

        // ambient light
        tWorld.setComponents(tWorld.newEntity("Ambient Light", EntityInterface::Light, scene),
                             AmbientLight(math::vec3(1.0f), 0.4));

        tWorld.setComponents(tWorld.newEntity("Cube", EntityInterface::Mesh, scene), Position(0.0f), Scale(1.0f), Orientation(),
                             Mesh(new Cube(1.0f), new BasicMaterial(BasicMaterialProps{.color = {0.5f, 0.5f, 0.5f}})));

        // tWorld.setComponents(tWorld.newEntity("Y Axis Arrow", EntityInterface::Mesh, scene), Position(0.0f, 0.0f, 0.0f),
        //                      Scale(1.0f), Orientation(),
        //                      Mesh(new CombinedGeometry(new Cylinder(0.01f, 0.6f, 4), new Cone(0.04f, 0.2f, 8, 0.6f)),
        //                           new PlainMaterial(PlainMaterialProps{.color = {0.0f, 1.0f, 0.0f}}), 1),
        //                      TranslationGizmo());

        // tWorld.setComponents(tWorld.newEntity("X Axis Arrow", EntityInterface::Mesh, scene), Position(0.0f, 0.0f, 0.0f),
        //                      Scale(1.0f), Orientation(math::angleAxis(-math::half_pi<float>(), math::vec3(0.0f, 0.0f, 1.0f))),
        //                      Mesh(new CombinedGeometry(new Cylinder(0.01f, 0.6f, 4), new Cone(0.04f, 0.2f, 8, 0.6f)),
        //                           new PlainMaterial(PlainMaterialProps{.color = {1.0f, 0.0f, 0.0f}}), 1),
        //                      TranslationGizmo());

        // tWorld.setComponents(tWorld.newEntity("Z Axis Arrow", EntityInterface::Mesh, scene), Position(0.0f, 0.0f, 0.0f),
        //                      Scale(1.0f), Orientation(math::angleAxis(math::half_pi<float>(), math::vec3(1.0f, 0.0f, 0.0f))),
        //                      Mesh(new CombinedGeometry(new Cylinder(0.01f, 0.6f, 4), new Cone(0.04f, 0.2f, 8, 0.6f)),
        //                           new PlainMaterial(PlainMaterialProps{.color = {0.0f, 0.0f, 1.0f}}), 1),
        //                      TranslationGizmo());

        // tWorld.setComponents(tWorld.newEntity("XZ plane", EntityInterface::Mesh, scene), Position(0.25f, 0.0f, 0.25f),
        //                      Scale(1.0f), Orientation(),
        //                      Mesh(new Plane(0.15f, 0.15f), new PlainMaterial(PlainMaterialProps{.color = {0.0f, 1.0f, 0.0f}}),
        //                      1), TranslationGizmo());

        // tWorld.setComponents(tWorld.newEntity("XY plane", EntityInterface::Mesh, scene), Position(0.24f, 0.25f, 0.0f),
        //                      Scale(1.0f), Orientation(math::angleAxis(-math::half_pi<float>(), math::vec3(1.0f, 0.0f, 0.0f))),
        //                      Mesh(new Plane(0.15f, 0.15f), new PlainMaterial(PlainMaterialProps{.color = {0.0f, 0.0f, 1.0f}}),
        //                      1), TranslationGizmo());

        // tWorld.setComponents(tWorld.newEntity("YZ plane", EntityInterface::Mesh, scene), Position(0.0f, 0.25f, 0.25f),
        //                      Scale(1.0f), Orientation(math::angleAxis(math::half_pi<float>(), math::vec3(0.0f, 0.0f, 1.0f))),
        //                      Mesh(new Plane(0.15f, 0.15f), new PlainMaterial(PlainMaterialProps{.color = {1.0f, 0.0f, 0.0f}}),
        //                      1), TranslationGizmo());

        window.registerListener<PointerMove>([&tWorld, cameraEntity](const PointerMove* event) {
            auto ray = windowClickToRay(tWorld, cameraEntity, {event->x, event->y}, {event->clientWidth, event->clientHeight});

            auto [cameraPosition] = tWorld.getComponents<Position>(cameraEntity);
            auto query = tWorld.query<Columns<Interactable>, Tags<>>();

            EntityID entityHit = 0;
            float entityHitDistanceToCamera = INFINITY;
            float entityHovered = false;

            std::vector<EntityID> hoverLostEntities;

            for (auto i = query.begin(); i != query.end(); i++) {
                auto currentEntityID = i.currentEntityID();

                auto [interactable] = *i;

                auto intersectionResult = interactable.boundingVolume->intersectRay(ray);

                if (!intersectionResult.has_value()) {
                    if (interactable.hovered) {
                        interactable.hovered = false;
                        hoverLostEntities.push_back(currentEntityID);
                    }
                    continue;
                }

                if (auto distanceToCamera = math::distance(intersectionResult.value(), static_cast<math::vec3>(cameraPosition));
                    distanceToCamera < entityHitDistanceToCamera) {
                    if (entityHit != 0 && entityHovered) {}
                    entityHit = currentEntityID;
                    entityHitDistanceToCamera = distanceToCamera;
                    entityHovered = interactable.hovered;
                }
            }

            if (entityHit != 0) {
                auto [interactable] = tWorld.getComponents<Interactable>(entityHit);

                interactable.hovered = true;
            }
        });

        // capturing tWorld here is okay since tWorld stays in the same location through the lifetime of the progam
        window.registerListener<WindowResize>([&tWorld, cameraEntity](const WindowResize* event) {
            auto [cameraSettings] = tWorld.getComponents<CameraSettings>(cameraEntity);

            // TODO: this should be reactive
            cameraSettings.aspectRatio = static_cast<float>(event->width) / static_cast<float>(event->height);

            // update the camera view
            auto [editorCameraController] = tWorld.getGlobal<CameraController>();
            editorCameraController.updateView(tWorld);
        });

        tWorld.setGlobal(std::move(window));
        tWorld.addSystems(SystemSchedule::Startup, gizmoInit);

        tWorld.addSystems(SystemSchedule::Update, windowUpdate, gizmoUpdate, renderSystem);
        // updating the window user pointer here, since the window is already in location that would stay permanent thought out
        // the lifetime of the application
        std::get<0>(tWorld.getGlobal<Window>()).updateUserPointer();
    }
};
