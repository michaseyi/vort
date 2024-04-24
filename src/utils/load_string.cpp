#include <string.h>
#include <cassert>
#include <fstream>
#include <sstream>

#if defined(EMSCRIPTEN)
#include <emscripten/emscripten.h>
#include <emscripten/fetch.h>
#endif

#if defined(EMSCRIPTEN)
EM_ASYNC_JS(char*, load_string_emscripten, (const char* path), {
  try {
    const dataPath = UTF8ToString(path);
    const data = new Uint8Array(await fetch(dataPath).then(
        function(res) { return res.arrayBuffer(); }));
    const length = data.length;
    const ptr = _malloc(length + 1);
    writeArrayToMemory(data, ptr);
    HEAP8.set(new Uint8Array([0]), ptr + length);
    return ptr;
  } catch {
    return 0;
  }
})
#endif

std::string load_string(std::string path) {
#if defined(EMSCRIPTEN)
  auto full_path = "/" + path;
  auto data = load_string_emscripten(full_path.c_str());
  assert(data && "Error fetching data");
  std::string result(data);
  free(data);

  // emscripten_fetch_attr_t attr;
  // emscripten_fetch_attr_init(&attr);
  // strcpy(attr.requestMethod, "GET");
  // attr.attributes =
  //     EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_SYNCHRONOUS;
  // emscripten_fetch_t* fetch = emscripten_fetch(&attr, path.c_str());
  // assert(fetch->data && "Error fetching data");
  // std::string result(fetch->data, fetch->data + fetch->numBytes);
  // emscripten_fetch_close(fetch);
  return result;
#else
  std::ifstream shader_file("/home/michaseyi/Projects/vort/" + path);
  std::stringstream ss;

  assert(shader_file.is_open());

  ss << shader_file.rdbuf();
  return ss.str();
#endif
}