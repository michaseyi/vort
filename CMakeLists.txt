cmake_minimum_required(VERSION 3.0...3.25)

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



if (NOT EMSCRIPTEN)
add_subdirectory(external/webgpu)
endif()

add_subdirectory(external/glfw3webgpu)
add_subdirectory(external/OpenMesh)

add_subdirectory(src/ecs)
add_subdirectory(src/renderer)
add_subdirectory(src/core)
add_subdirectory(src/utils)
add_subdirectory(src/math)



target_link_libraries(Vort PRIVATE  glfw3webgpu vort-ecs vort-core vort-renderer vort-math vort-utils OpenMeshCore)

if(NOT EMSCRIPTEN)
   target_link_libraries(Vort PRIVATE  webgpu glfw xcb X11 X11-xcb)
else()
    target_link_options(Vort PRIVATE -sUSE_GLFW=3 -sUSE_WEBGPU -sASYNCIFY -sFILESYSTEM=0 -sSTB_IMAGE=1)
    set_target_properties(Vort PROPERTIES SUFFIX ".html")
    
    # target_link_options(App PRIVATE -sUSE_GLFW=3 -sUSE_WEBGPU  -sMODULARIZIE -sALLOW_MEMORY_GROWTH -sASYNCIFY -sFETCH  -sENVIRONMENT=web -sEXPORT_ES6=1 -lembind -sFILESYSTEM=0 -sMALLOC=emmalloc)
    # set_target_properties(App PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/vort-editor/src/emscripten)
endif()
