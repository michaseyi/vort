// #include <GLFW/glfw3.h>
// #ifndef EMSCRIPTEN
// #define EMSCRIPTEN
// #endif

// #if defined(EMSCRIPTEN)
// #include <emscripten/bind.h>
// #include <emscripten/emscripten.h>
// #include <emscripten/fetch.h>
// #endif

// #include <string.h>
// #include <webgpu/webgpu.h>

// #include <cassert>
// #include <cstdint>
// #include <filesystem>
// #include <fstream>
// #include <functional>
// #include <iostream>
// #include <memory>
// #include <sstream>
// #include <vector>
// #if !defined(EMSCRIPTEN)
// #include "glfw3webgpu/glfw3webgpu.h"
// #endif

// #include "vort-ecs/ecs.hpp"
// #include "vort-renderer/src/Context.hpp"
// #include "vort-renderer/src/Uniform.hpp"

// using World = Entities;

// template <typename T>
// struct Vector3 {
//     union {
//         struct {
//             T x;
//             T y;
//             T z;
//         };
//         T data[3];
//     };

//     void setX(T value) { x = value; };
//     void setY(T value) { y = value; };
//     void setZ(T value) { z = value; };

//     T getX() const {
//         std::cout << this << std::endl;
//         return x;
//     };
//     T getY() const { return y; };
//     T getZ() const { return z; };
// };

// EM_JS(int, getDisplayCanvasWidth, (), { return document.querySelector("#canvas").width; });

// EM_JS(int, getDisplayCanvasHeight, (), { return document.querySelector("#canvas").height; });

// struct Mouse {
//     double prevX;
//     double prevY;
//     double currentX;
//     double currentY;
// };

// struct Position : Vector3<float> {
//     // static Vector3<float> *getComponent(EntityID entityID) { return &std::get<0>(App().getComponents<Position>(entityID)); };
// };

// struct Velocity : Vector3<float> {
//     // static Vector3<float> *getComponent(EntityID entityID) { return &std::get<0>(App().getComponents<Velocity>(entityID)); };
// };

// WGPUSurface getHTLMCanvasSurface(WGPUInstance instance, std::string selector) {
//     WGPUSurfaceDescriptorFromCanvasHTMLSelector canvas;
//     canvas.chain = {.next = nullptr, .sType = WGPUSType_SurfaceDescriptorFromCanvasHTMLSelector};
//     canvas.selector = selector.c_str();

//     WGPUSurfaceDescriptor surfaceDesc;
//     surfaceDesc.label = nullptr;
//     surfaceDesc.nextInChain = (const WGPUChainedStruct *)&canvas;

//     return wgpuInstanceCreateSurface(instance, &surfaceDesc);
// }

// struct VertexBuffer {
//     WGPUBuffer buffer;
//     WGPUVertexBufferLayout layout;
//     size_t vertexCount;
//     size_t size;
//     std::vector<WGPUVertexAttribute> vertexAttributes;

//     VertexBuffer() = default;

//     VertexBuffer &operator=(VertexBuffer &&rhs) {
//         buffer = rhs.buffer;
//         layout = rhs.layout;
//         vertexCount = rhs.vertexCount;
//         size = rhs.size;
//         vertexAttributes = std::move(rhs.vertexAttributes);
//         rhs.valid = false;
//         return *this;
//     }
//     VertexBuffer(VertexBuffer &&rhs) {
//         buffer = rhs.buffer;
//         layout = rhs.layout;
//         vertexCount = rhs.vertexCount;
//         size = rhs.size;
//         vertexAttributes = std::move(rhs.vertexAttributes);
//         rhs.valid = false;
//     }

//     ~VertexBuffer() {
//         if (valid) {
//             wgpuBufferDestroy(buffer);
//             wgpuBufferRelease(buffer);
//         }
//     }

// private:
//     bool valid = true;
// };

// #if defined(EMSCRIPTEN)
// EM_ASYNC_JS(char *, loadString, (const char *path), {
//     const dataPath = UTF8ToString(path);
//     const data = await fetch(dataPath).then(function(res) { return res.text(); });
//     const length = lengthBytesUTF8(data);
//     const ptr = _malloc(length);
//     stringToUTF8(data, ptr, length);
//     return ptr;
// })

// std::string loadFileFromServer(std::string path) {
//     auto fullPath = "/" + path;
//     auto data = loadString(fullPath.c_str());
//     std::string result = data;
//     free(data);
//     return result;
// }
// #endif

// std::string loadFileFromFs(std::filesystem::path path) {
//     std::ifstream shaderFile{path};

//     std::stringstream ss;

//     assert(shaderFile.is_open());
//     ss << shaderFile.rdbuf();
//     return ss.str();
// }

// std::string loadFile(std::string &path) {
// #if defined(EMSCRIPTEN)
//     return loadFileFromServer(path);
// #else
//     return loadFileFromFs(path);
// #endif
// }

// std::string preprocessShader(std::string &content) {
//     std::stringstream current_shader(content);
//     std::stringstream ss;

//     std::string line_buffer;
//     while (!current_shader.eof()) {
//         std::getline(current_shader, line_buffer);
//         if (line_buffer.starts_with("#import")) {
//             auto start = line_buffer.find_first_of("\"");
//             auto end = line_buffer.find_last_of("\"");

//             assert(start != std::string::npos && end != std::string::npos);
//             std::string path = line_buffer.substr(start + 1, end - start - 1) + ".wgsl";

//             std::string rawSrc = loadFile(path);
//             ss << preprocessShader(rawSrc);

//         } else {
//             ss << line_buffer << "\n";
//         }
//     }
//     return ss.str();
// }

// WGPUShaderModule createShaderModule(WGPUDevice device, std::string &&sourceCodePath) {
//     std::string rawSrc = loadFile(sourceCodePath);
//     std::string sourceCode = preprocessShader(rawSrc);

//     WGPUShaderModuleWGSLDescriptor shaderModuleCode{};
//     shaderModuleCode.chain.next = nullptr;
//     shaderModuleCode.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;
//     shaderModuleCode.code = sourceCode.c_str();

//     WGPUShaderModuleDescriptor shaderModuleDesc{};
//     shaderModuleDesc.label = "Default Shader";
//     shaderModuleDesc.nextInChain = &shaderModuleCode.chain;

//     WGPUShaderModule shaderModule = wgpuDeviceCreateShaderModule(device, &shaderModuleDesc);

//     assert(shaderModule);
//     return shaderModule;
// }

// VertexBuffer createExampleVertexBuffer(WGPUDevice device) {
//     std::vector<float> vertexData = {-0.5, -0.5, 1.0, 0.0, 0.0, 0.5, -0.5, 0.0, 1.0, 0.0, 0.0, 0.5, 0.0, 0.0, 1.0};

//     VertexBuffer buffer{};
//     buffer.vertexCount = vertexData.size() / 5;
//     buffer.size = sizeof(float) * vertexData.size();

//     WGPUBufferDescriptor bufferDesc{};
//     bufferDesc.label = "Vertex Buffer";
//     bufferDesc.mappedAtCreation = false;
//     bufferDesc.nextInChain = nullptr;
//     bufferDesc.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Vertex;
//     bufferDesc.size = buffer.size;

//     auto wgpuBuffer = wgpuDeviceCreateBuffer(device, &bufferDesc);
//     assert(wgpuBuffer);

//     WGPUVertexAttribute positionAttribute{};
//     positionAttribute.offset = 0;
//     positionAttribute.shaderLocation = 0;
//     positionAttribute.format = WGPUVertexFormat_Float32x2;
//     buffer.vertexAttributes.push_back(positionAttribute);

//     WGPUVertexAttribute colorAttribute{};
//     colorAttribute.offset = sizeof(float) * 2;
//     colorAttribute.shaderLocation = 1;
//     colorAttribute.format = WGPUVertexFormat_Float32x3;
//     buffer.vertexAttributes.push_back(colorAttribute);

//     WGPUVertexBufferLayout layout{};
//     layout.attributeCount = buffer.vertexAttributes.size();
//     layout.attributes = buffer.vertexAttributes.data();
//     layout.stepMode = WGPUVertexStepMode_Vertex;
//     layout.arrayStride = sizeof(float) * 5;

//     buffer.buffer = wgpuBuffer;
//     buffer.layout = layout;

//     auto queue = wgpuDeviceGetQueue(device);

//     wgpuQueueWriteBuffer(queue, wgpuBuffer, 0, vertexData.data(), sizeof(float) * vertexData.size());
//     return buffer;
// }

// struct Shader {
//     WGPUShaderModule shaderModule;
//     Shader() = default;

//     Shader(WGPUShaderModule module) : shaderModule(module){};

//     Shader(Shader &&rhs) {
//         shaderModule = rhs.shaderModule;
//         rhs.valid = false;
//     }
//     Shader &operator=(Shader &&rhs) {
//         shaderModule = rhs.shaderModule;
//         rhs.valid = false;
//         return *this;
//     }

//     ~Shader() {
//         if (valid) {
//             wgpuShaderModuleRelease(shaderModule);
//         }
//     }

// private:
//     bool valid = true;
// };

// struct WindowContext {
//     GLFWwindow *window;
//     int width;
//     int height;

//     WindowContext() = default;

//     WindowContext(WindowContext &&rhs) {
//         window = rhs.window;
//         width = rhs.width;
//         height = rhs.height;
//         rhs.valid = false;
//     }

//     WindowContext &operator=(WindowContext &&rhs) {
//         window = rhs.window;
//         width = rhs.width;
//         height = rhs.height;
//         rhs.valid = false;
//         return *this;
//     }

//     ~WindowContext() {
//         if (valid) {
//             glfwDestroyWindow(window);
//             glfwTerminate();
//         }
//     }

// private:
//     bool valid = true;
// };

// struct General {
//     int32_t resolution[2];
//     float time;
//     char __padding[4];
// } __attribute__((packed));

// void wgpuContextSetup(Command &command, Global<WindowContext> &global) {
//     auto [windowContext] = global;

//     auto wgpuInitStatus = Context::init();

//     assert(wgpuInitStatus);

// #if defined(EMSCRIPTEN)
//     context.surface = getHTLMCanvasSurface(context.instance, "#canvas");
// #else
//     context.surface = glfwGetWGPUSurface(context.instance, windowContext.window);
// #endif
//     assert(context.surface != nullptr);

//     WGPURequestAdapterOptions adapterOptions{};
//     adapterOptions.nextInChain = nullptr;
//     adapterOptions.compatibleSurface = context.surface;

//     context.adapter = requestAdapter(context.instance, &adapterOptions);
//     assert(context.adapter != nullptr);

//     WGPUDeviceDescriptor deviceDesc{};
//     deviceDesc.nextInChain = nullptr;
//     deviceDesc.label = "My Device";
//     deviceDesc.requiredFeaturesCount = 0;
//     deviceDesc.requiredFeatures = nullptr;
//     deviceDesc.requiredLimits = nullptr;
//     deviceDesc.defaultQueue.nextInChain = nullptr;
//     deviceDesc.defaultQueue.label = "The default queue";

//     WGPUTextureFormat swapChainFormat = WGPUTextureFormat_BGRA8Unorm;
//     WGPUSwapChainDescriptor swapChainDesc = {};
//     swapChainDesc.nextInChain = nullptr;
//     swapChainDesc.label = "My SwapChain";
//     swapChainDesc.width = windowContext.width;
//     swapChainDesc.height = windowContext.height;
//     swapChainDesc.usage = WGPUTextureUsage_RenderAttachment;
//     swapChainDesc.format = swapChainFormat;
//     swapChainDesc.presentMode = WGPUPresentMode_Fifo;
//     context.swapChain = wgpuDeviceCreateSwapChain(context.device, context.surface, &swapChainDesc);
//     context.swapChainFormat = swapChainFormat;
//     assert(context.swapChain);

//     auto generalUniform = std::make_shared<Uniform<General>>(Uniform<General>(1, false));

//     std::cout << windowContext.width << " " << windowContext.height << std::endl;

//     auto generalUniformData = General{.resolution = {windowContext.width, windowContext.height}, .time = static_cast<float>(glfwGetTime())};

//     generalUniform->set(generalUniformData);

//     command.setGlobal(std::move(generalUniform));
//     command.setGlobal(generalUniformData);
//     command.setGlobal(std::move(context));
// }

// void glfwWindowContextSetup(Command &command) {
//     WindowContext windowContext;

//     int width = 500;
//     int height = 500;

// #if defined(EMSCRIPTEN)
//     width = getDisplayCanvasWidth();
//     height = getDisplayCanvasHeight();
// #endif

//     int glfwInitStatus = glfwInit();
//     assert(glfwInitStatus);

//     // glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
//     glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

//     windowContext.window = glfwCreateWindow(width, height, "learn WebGPU", nullptr, nullptr);
//     assert(windowContext.window != nullptr);

//     glfwGetWindowSize(windowContext.window, &windowContext.width, &windowContext.height);

//     std::cout << windowContext.width << " : " << windowContext.height << std::endl;
//     Mouse mouse;
//     glfwGetCursorPos(windowContext.window, &mouse.prevX, &mouse.prevY);
//     mouse.currentX = mouse.prevX;
//     mouse.currentY = mouse.prevY;

//     command.setGlobal(std::move(windowContext));
//     command.setGlobal(std::move(mouse));
//     auto [savedWindowContext] = command.getGlobal<WindowContext>();
// }

// void exampleTriangleSetup(Command &command, Global<WGPUContext> &global) {
//     auto [wgpuContext] = global;

//     command.spawn(Position{20.0f, 1.0f, 22.0f}, Velocity{2.0f, 19.0f, 100.0f}, createExampleVertexBuffer(wgpuContext.device),
//                   Shader{createShaderModule(wgpuContext.device, "shader.wgsl")});
// }

// void windowUpdate(Global<WindowContext, AppState, General> &global) {
//     auto [windowContext, appState, general] = global;

//     general.time = glfwGetTime();

//     glfwPollEvents();

//     if (glfwWindowShouldClose(windowContext.window)) {
//         appState.running = false;
//     }
// }

// void renderSystem(Query<Columns<VertexBuffer, Shader, Position, Velocity>, Tags<>> &query,
//                   Global<WGPUContext, UniformStorage<General>, General> &global) {
//     auto [wgpuContext, generalUniformStorage, general] = global;

// #if !defined(EMSCRIPTEN)
//     wgpuDeviceTick(wgpuContext.device);
// #endif

//     WGPUCommandEncoderDescriptor commandEncoderDescriptor{};
//     commandEncoderDescriptor.nextInChain = nullptr;
//     commandEncoderDescriptor.label = "My Command Encoder";

//     WGPUCommandEncoder commandEncoder = wgpuDeviceCreateCommandEncoder(wgpuContext.device, &commandEncoderDescriptor);

//     assert(commandEncoder != nullptr);

//     WGPUTextureView nextTexture = wgpuSwapChainGetCurrentTextureView(wgpuContext.swapChain);

//     // next swap chain texture;

//     // actual rendering operations

//     // color attachment for render pass
//     WGPURenderPassColorAttachment renderPassColorAttachment{};
//     renderPassColorAttachment.view = nextTexture;
//     renderPassColorAttachment.clearValue = WGPUColor{0.4, 0.4, 0.4, 0.0};
//     renderPassColorAttachment.nextInChain = nullptr;
//     renderPassColorAttachment.resolveTarget = nullptr;
//     renderPassColorAttachment.loadOp = WGPULoadOp_Clear;
//     renderPassColorAttachment.storeOp = WGPUStoreOp_Store;

//     WGPURenderPassDescriptor renderPassDesc{};
//     renderPassDesc.colorAttachmentCount = 1;
//     renderPassDesc.colorAttachments = &renderPassColorAttachment;
//     renderPassDesc.depthStencilAttachment = nullptr;
//     renderPassDesc.nextInChain = nullptr;
// #if !defined(EMSCRIPTEN)
//     renderPassDesc.timestampWriteCount = 0;
// #endif
//     renderPassDesc.timestampWrites = nullptr;

//     WGPURenderPassEncoder renderPassEncoder = wgpuCommandEncoderBeginRenderPass(commandEncoder, &renderPassDesc);

//     generalUniformStorage.set(general);

//     for (auto [vertexBuffer, shader, position, velocity] : query) {
// #pragma region bindgroup

//         WGPUBindGroupLayoutEntry bindingLayout = generalUniformStorage;
//         bindingLayout.binding = 0;

//         WGPUBindGroupLayoutDescriptor bindGroupLayoutDesc{};
//         bindGroupLayoutDesc.nextInChain = nullptr;
//         bindGroupLayoutDesc.entries = &bindingLayout;
//         bindGroupLayoutDesc.entryCount = 1;

//         WGPUBindGroupLayout bindGroupLayout = wgpuDeviceCreateBindGroupLayout(wgpuContext.device, &bindGroupLayoutDesc);

//         WGPUPipelineLayoutDescriptor layoutDesc{};
//         layoutDesc.nextInChain = nullptr;
//         layoutDesc.bindGroupLayoutCount = 1;
//         layoutDesc.bindGroupLayouts = &bindGroupLayout;
//         WGPUPipelineLayout layout = wgpuDeviceCreatePipelineLayout(wgpuContext.device, &layoutDesc);

//         WGPUBindGroupEntry binding = generalUniformStorage;
//         binding.binding = 0;

//         WGPUBindGroupDescriptor bindGroupDesc{};
//         bindGroupDesc.entries = &binding;
//         bindGroupDesc.entryCount = 1;
//         bindGroupDesc.layout = bindGroupLayout;

//         WGPUBindGroup bindGroup = wgpuDeviceCreateBindGroup(wgpuContext.device, &bindGroupDesc);
// #pragma endregion

// #pragma region
//         WGPURenderPipelineDescriptor renderPipelineDesc{};
//         renderPipelineDesc.label = "Main Pipeline";

//         // vertex state
//         renderPipelineDesc.vertex.bufferCount = 1;
//         renderPipelineDesc.vertex.buffers = &vertexBuffer.layout;
//         renderPipelineDesc.vertex.module = shader.shaderModule;
//         renderPipelineDesc.vertex.entryPoint = "vs_main";
//         renderPipelineDesc.vertex.constants = nullptr;
//         renderPipelineDesc.vertex.constantCount = 0;
//         renderPipelineDesc.vertex.nextInChain = nullptr;

//         // primitives state
//         renderPipelineDesc.primitive.frontFace = WGPUFrontFace_CCW;
//         renderPipelineDesc.primitive.cullMode = WGPUCullMode_Back;
//         renderPipelineDesc.primitive.topology = WGPUPrimitiveTopology_TriangleList;
//         renderPipelineDesc.primitive.stripIndexFormat = WGPUIndexFormat_Undefined;
//         renderPipelineDesc.nextInChain = nullptr;

//         // fragment state
//         WGPUFragmentState fragmentState{};

//         WGPUBlendState blendState{};
//         blendState.color.srcFactor = WGPUBlendFactor_SrcAlpha;
//         blendState.color.operation = WGPUBlendOperation_Add;
//         blendState.color.dstFactor = WGPUBlendFactor_OneMinusSrcAlpha;

//         blendState.alpha.srcFactor = WGPUBlendFactor_SrcAlpha;
//         blendState.alpha.operation = WGPUBlendOperation_Add;
//         blendState.alpha.dstFactor = WGPUBlendFactor_OneMinusSrcAlpha;

//         WGPUColorTargetState colorTargetState{};
//         colorTargetState.format = wgpuContext.swapChainFormat;
//         colorTargetState.nextInChain = nullptr;
//         colorTargetState.writeMask = WGPUColorWriteMask_All;
//         colorTargetState.blend = &blendState;

//         fragmentState.nextInChain = nullptr;
//         fragmentState.module = shader.shaderModule;
//         fragmentState.entryPoint = "fs_main";
//         fragmentState.constantCount = 0;
//         fragmentState.constants = nullptr;
//         fragmentState.targetCount = 1;
//         fragmentState.targets = &colorTargetState;
//         renderPipelineDesc.fragment = &fragmentState;

//         // depth and stencil
//         renderPipelineDesc.depthStencil = nullptr;

//         renderPipelineDesc.multisample.count = 1;
//         renderPipelineDesc.multisample.mask = ~0u;
//         renderPipelineDesc.multisample.nextInChain = nullptr;
//         renderPipelineDesc.multisample.alphaToCoverageEnabled = false;

// #pragma endregion

//         renderPipelineDesc.layout = layout;
//         WGPURenderPipeline renderPipeline = wgpuDeviceCreateRenderPipeline(wgpuContext.device, &renderPipelineDesc);

//         wgpuRenderPassEncoderSetBindGroup(renderPassEncoder, 0, bindGroup, 0, nullptr);

//         wgpuRenderPassEncoderSetPipeline(renderPassEncoder, renderPipeline);
//         wgpuRenderPassEncoderSetVertexBuffer(renderPassEncoder, 0, vertexBuffer.buffer, 0, vertexBuffer.size);

//         wgpuRenderPassEncoderDraw(renderPassEncoder, vertexBuffer.vertexCount, 1, 0, 0);
//         wgpuRenderPipelineRelease(renderPipeline);
//     }

//     wgpuRenderPassEncoderEnd(renderPassEncoder);

//     WGPUCommandBufferDescriptor commandBufferDesc{};
//     commandBufferDesc.nextInChain = nullptr;
//     commandBufferDesc.label = "My Command Buffer";
//     WGPUCommandBuffer commandBuffer = wgpuCommandEncoderFinish(commandEncoder, &commandBufferDesc);

//     wgpuQueueSubmit(wgpuContext.queue, 1, &commandBuffer);

// #if !defined(EMSCRIPTEN)
//     wgpuSwapChainPresent(wgpuContext.swapChain);
// #endif

//     wgpuRenderPassEncoderRelease(renderPassEncoder);
//     wgpuTextureViewRelease(nextTexture);
//     wgpuCommandBufferRelease(commandBuffer);
//     wgpuCommandEncoderRelease(commandEncoder);
// }

// int mai() {
//     World()
//         .addSystems(SystemSchedule::Startup, glfwWindowContextSetup, wgpuContextSetup, exampleTriangleSetup)
//         .addSystems(SystemSchedule::Update, windowUpdate, renderSystem)
//         .run();
// }

// #if defined(EMSCRIPTEN)
// using namespace emscripten;

// std::vector<int> shout() { return {1, 2, 3}; }

// EMSCRIPTEN_BINDINGS(bindings) {
//     class_<Vector3<float>>("Vector3Float")
//         .constructor()
//         .property("x", &Vector3<float>::getX, &Vector3<float>::setX)
//         .property("y", &Vector3<float>::getY, &Vector3<float>::setY)
//         .property("z", &Vector3<float>::getZ, &Vector3<float>::setZ);

//     // function("getPositionComponent", &Position::getComponent, allow_raw_pointers());
//     // function("getVelocityComponent", &Velocity::getComponent, allow_raw_pointers());
//     function("shout", &shout);
//     register_vector<int>("vector<int>");
// }
// #endif
