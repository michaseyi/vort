#include <string.h>
#include <cassert>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

#if defined(EMSCRIPTEN)
#include <emscripten/emscripten.h>
#include <emscripten/fetch.h>
#endif

#if defined(EMSCRIPTEN)

EM_ASYNC_JS(uint8_t*, load_buffer_emscripten,
            (const char* path, uint32_t* lengthPtr), {
              const dataPath = UTF8ToString(path);
              try {
                const data = new Uint8Array(await fetch(dataPath).then(
                    function(res) { return res.arrayBuffer(); }));
                const length = data.length;
                const ptr = _malloc(length);

                HEAPU32.set(new Uint32Array([length]), (lengthPtr / 4) | 0);

                writeArrayToMemory(data, ptr);
                return ptr;
              } catch {
                HEAPU32.set(new Uint32Array([0]), (lengthPtr / 4) | 0);
                return 0;
              }
            })
#endif

std::vector<uint8_t> load_buffer(std::string path) {
#if defined(EMSCRIPTEN)
  auto full_path = "/" + path;
  uint32_t length = 0;
  auto data = load_buffer_emscripten(full_path.c_str(), &length);
  std::vector<uint8_t> result(data, data + length);

  free(data);

  // emscripten_fetch_attr_t attr;
  // emscripten_fetch_attr_init(&attr);
  // strcpy(attr.requestMethod, "GET");
  // attr.attributes =
  //     EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_SYNCHRONOUS;
  // emscripten_fetch_t* fetch = emscripten_fetch(&attr, path.c_str());
  // assert(fetch->data && "Error fetching data");

  // std::vector<uint8_t> result(fetch->data, fetch->data + fetch->numBytes);
  // emscripten_fetch_close(fetch);
  return result;
#else
  std::ifstream file_stream(path);
  std::vector<uint8_t> result;

  assert(file_stream.is_open());

  result.assign(std::istreambuf_iterator<char>(file_stream),
                std::istreambuf_iterator<char>());
  return result;
#endif
}