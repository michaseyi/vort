#pragma once
#if defined(__EMSCRIPTEN__)
#include "webgpu_emscripten.hpp"
#else
#include "webgpu_dawn.hpp"
#endif