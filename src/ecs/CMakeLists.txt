cmake_minimum_required(VERSION 3.2)

add_library(vort-ecs)

file(GLOB_RECURSE VORT_ECS_SOURCES *.cpp)

target_sources(vort-ecs PRIVATE ${VORT_ECS_SOURCES})
