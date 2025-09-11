#pragma once

constexpr static unsigned int IMAGE_LOAD_GRAY = 0x0001U;
constexpr static unsigned int IMAGE_LOAD_FLIP = 0x0002U;

struct Image final {
    static void register_resource(void);

    stbi_uc* pixels;
    glm::ivec2 size;
};
