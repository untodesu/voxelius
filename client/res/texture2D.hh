#ifndef BBB0B8F1_CB2A_42FD_9123_7D5E5339FFBE
#define BBB0B8F1_CB2A_42FD_9123_7D5E5339FFBE

#include "client/gpu/texture.hh"

constexpr static std::uint32_t RESFLAG_TEX_FLIP = 1 << 8; ///< Flip image vertically on load
constexpr static std::uint32_t RESFLAG_TEX_GRAY = 1 << 9; ///< If set, the pixel data is 8-bit grayscale, otherwise it's RGBA8888

struct Texture2D final {
    static void register_resource(void);

    gpu::Texture gpu;
    ImTextureID imgui;
};

#endif /* BBB0B8F1_CB2A_42FD_9123_7D5E5339FFBE */
