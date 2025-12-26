// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: texture_gui.hh
// Description: GUI texture resource

#ifndef CLIENT_RESOURCE_TEXTURE_GUI_HH
#define CLIENT_RESOURCE_TEXTURE_GUI_HH
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

#endif
