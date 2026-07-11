#ifndef D5BF2E67_1DC9_480C_A917_00BD401ACB86
#define D5BF2E67_1DC9_480C_A917_00BD401ACB86

#include "client/gpu/handle.hh"

namespace gpu
{
// Pass to any create_* num_levels argument to have the
// full mip chain size computed from width/height automatically
constexpr static int MIP_LEVELS_AUTO = -1;

class Texture final : public Handle<SDL_GPUTexture, SDL_ReleaseGPUTexture> {
public:
    Texture(void) = default;

    static Texture create(const SDL_GPUTextureCreateInfo& info);
    static Texture create_2D(Uint32 width, Uint32 height, SDL_GPUTextureFormat format, SDL_GPUTextureUsageFlags usage,
        int num_levels = MIP_LEVELS_AUTO);
    static Texture create_cube(Uint32 size, SDL_GPUTextureFormat format, SDL_GPUTextureUsageFlags usage, int num_levels = MIP_LEVELS_AUTO);
    static Texture create_array(Uint32 width, Uint32 height, Uint32 layers, SDL_GPUTextureFormat format, SDL_GPUTextureUsageFlags usage,
        int num_levels = MIP_LEVELS_AUTO);

    constexpr Uint32 width(void) const;
    constexpr Uint32 height(void) const;
    constexpr SDL_GPUTextureFormat format(void) const;

    // width/height default to 0, meaning "the whole texture from (x, y)"
    void write_streamed(SDL_GPUCopyPass* copy_pass, std::span<const std::byte> data, Uint32 layer = 0, Uint32 mip_level = 0, Uint32 x = 0,
        Uint32 y = 0, Uint32 width = 0, Uint32 height = 0);
    void write_blocking(std::span<const std::byte> data, Uint32 layer = 0, Uint32 mip_level = 0, Uint32 x = 0, Uint32 y = 0,
        Uint32 width = 0, Uint32 height = 0);
    void generate_mipmaps_blocking(void) const;

private:
    explicit Texture(SDL_GPUDevice* device, SDL_GPUTexture* handle, Uint32 width, Uint32 height, SDL_GPUTextureFormat format);

    Uint32 m_width { 0 };
    Uint32 m_height { 0 };
    SDL_GPUTextureFormat m_format { SDL_GPU_TEXTUREFORMAT_INVALID };
};
} // namespace gpu

constexpr Uint32 gpu::Texture::width(void) const
{
    return m_width;
}

constexpr Uint32 gpu::Texture::height(void) const
{
    return m_height;
}

constexpr SDL_GPUTextureFormat gpu::Texture::format(void) const
{
    return m_format;
}

#endif /* D5BF2E67_1DC9_480C_A917_00BD401ACB86 */
