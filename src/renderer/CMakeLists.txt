cmake_minimum_required(VERSION 3.0...3.25)

file(GLOB_RECURSE SOURCES *.cpp)

add_library(vort-renderer ${SOURCES})


set_target_properties(vort-renderer PROPERTIES
    CXX_STANDARD 23
)




