

#include "src/utils/raii.hpp"
#include "wgpu_context.hpp"



class Texture {
public:
    Texture() = default;

    static Texture createCubeMap();
    static Texture create2DTexture(std::string tPath);

private:
    RAIIWrapper<wgpu::Texture> mWGPUTexture;
};