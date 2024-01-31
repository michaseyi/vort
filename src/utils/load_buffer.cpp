#include <cassert>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

#if defined(EMSCRIPTEN)
#include <emscripten/emscripten.h>
#endif

#if defined(EMSCRIPTEN)
EM_ASYNC_JS(uint8_t *, loadBufferEmscripten, (const char *path, uint32_t *lengthPtr), {
    const dataPath = UTF8ToString(path);
    try {
        const data = new Uint8Array(await fetch(dataPath).then(function(res) { return res.arrayBuffer(); }));
        const length = data.length;
        const ptr = _malloc(length);

        HEAPU32.set(new Uint32Array([length]), (lengthPtr / 4) | 0);

        writeArrayToMemory(data, ptr);
        return ptr;
    } catch  {
        HEAPU32.set(new Uint32Array([0]), (lengthPtr / 4) | 0);
        return 0;
    }
})
#endif

std::vector<uint8_t> loadBuffer(std::string tPath) {
#if defined(EMSCRIPTEN)
    auto fullPath = "/" + tPath;
    uint32_t length = 0;
    auto data = loadBufferEmscripten(fullPath.c_str(), &length);

    assert(data && "Error fetching data");

    std::vector<uint8_t> result(data, data + length);

    free(data);
    return result;
#else
    std::ifstream fileStream(tPath);
    std::vector<uint8_t> result;

    assert(fileStream.is_open());

    result.assign(std::istreambuf_iterator<char>(fileStream), std::istreambuf_iterator<char>());
    return result;
#endif
}