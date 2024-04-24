cmake_minimum_required(VERSION 3.0...3.25)

# add_compile_options(-pthread)
project(Vort
        VERSION 0.1
        LANGUAGES CXX C)  


if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()


add_executable(Vort src/main.cpp)

target_compile_options(Vort PUBLIC -Wall -Wextra -pedantic)

set_target_properties(Vort PROPERTIES
    CXX_STANDARD 20
    CXX_EXTENSIONS OFF
    COMPILE_WARNING_AS_ERROR ON
)


include_directories(external ${PROJECT_SOURCE_DIR} external/OpenMesh/src)


add_subdirectory(external/glfw3webgpu)

if (NOT EMSCRIPTEN)
add_subdirectory(external/webgpu)
endif()

if (EMSCRIPTEN)
add_subdirectory(src/bindings)
endif()

add_subdirectory(external/OpenMesh)

add_subdirectory(src/ecs)
add_subdirectory(src/renderer)
add_subdirectory(src/core)
add_subdirectory(src/utils)
add_subdirectory(src/math)



target_link_libraries(Vort PRIVATE   vort-ecs vort-core vort-renderer vort-math vort-utils OpenMeshCore  glfw3webgpu)

if(NOT EMSCRIPTEN)
   target_link_libraries(Vort PRIVATE  webgpu glfw xcb X11 X11-xcb glfw3webgpu)
else()
    # target_link_options(Vort PRIVATE -sUSE_GLFW=3 -sUSE_WEBGPU -sASYNCIFY -sFILESYSTEM=0 -sSTB_IMAGE=1 -sALLOW_MEMORY_GROWTH -sENVIRONMENT=web  -sMALLOC=emmalloc -lembind  -sEXPORTED_RUNTIME_METHODS=[UTF8ToString,stringToUTF8])
    # set_target_properties(Vort PROPERTIES SUFFIX ".html")
    
    target_link_libraries(Vort PRIVATE  vort-bindings)
    target_link_options(Vort PRIVATE -sUSE_GLFW=3 -sUSE_WEBGPU -sASYNCIFY -sFILESYSTEM=0 -sSTB_IMAGE=1 -sALLOW_MEMORY_GROWTH -sENVIRONMENT=web,worker   -sASSERTIONS  -lembind -sEXPORT_NAME="Vort" -sMODULARIZE -sEXPORTED_RUNTIME_METHODS=[UTF8ToString,Asyncify,wasmMemory,stringToNewUTF8,wasmExports])
    set_target_properties(Vort PROPERTIES RUNTIME_OUTPUT_DIRECTORY /home/michaseyi/Projects/vort-editor/public)
endif()
