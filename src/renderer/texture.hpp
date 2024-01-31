

#include "src/utils/raii.hpp"
#include "wgpu_context.hpp"

template <>
struct RAIIDeleter<wgpu::Texture> {
    void deleter(wgpu::Texture &tTexture) {
        tTexture.destroy();
        tTexture.release();
    }
};

class Texture {
public:
    Texture() = default;

    static Texture createCubeMap();
    static Texture create2DTexture(std::string tPath);

private:
    RAIIWrapper<wgpu::Texture> mWGPUTexture;
};