#include <cassert>
#include <fstream>
#include <sstream>

#if defined(EMSCRIPTEN)
#include <emscripten/emscripten.h>
#endif

#if defined(EMSCRIPTEN)
EM_ASYNC_JS(char *, loadStringEmscripten, (const char *path), {
    try {
        const dataPath = UTF8ToString(path);
        const data = new Uint8Array(await fetch(dataPath).then(function(res) { return res.arrayBuffer(); }));
        const length = data.length;
        const ptr = _malloc(length + 1);
        writeArrayToMemory(data, ptr);
        HEAP8.set(new Uint8Array([0]), ptr + length);
        return ptr;
    } catch  {
        return 0;
    }
})
#endif

std::string loadString(std::string tPath) {
#if defined(EMSCRIPTEN)
    auto fullPath = "/" + tPath;
    auto data = loadStringEmscripten(fullPath.c_str());

    assert(data && "Error fetching data");

    std::string result = data;
    free(data);
    return result;
#else
    std::ifstream shaderFile(tPath);
    std::stringstream ss;

    assert(shaderFile.is_open());

    ss << shaderFile.rdbuf();
    return ss.str();
#endif
}