#ifndef CLIENT_TEXTURE2D_HH
#define CLIENT_TEXTURE2D_HH 1
#pragma once

constexpr static unsigned int TEXTURE_GUI_LOAD_CLAMP_S = 0x0001;
constexpr static unsigned int TEXTURE_GUI_LOAD_CLAMP_T = 0x0002;
constexpr static unsigned int TEXTURE_GUI_LOAD_LINEAR_MAG = 0x0004;
constexpr static unsigned int TEXTURE_GUI_LOAD_LINEAR_MIN = 0x0008;
constexpr static unsigned int TEXTURE_GUI_LOAD_VFLIP = 0x0010;
constexpr static unsigned int TEXTURE_GUI_LOAD_GRAYSCALE = 0x0020;

struct TextureGUI final {
    static void register_resource(void);

    ImTextureID handle;
    glm::ivec2 size;
};

#endif // CLIENT_TEXTURE2D_HH
