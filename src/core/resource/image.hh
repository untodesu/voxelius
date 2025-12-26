// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: image.hh
// Description: Image resource

#ifndef CORE_RESOURCE_IMAGE_HH
#define CORE_RESOURCE_IMAGE_HH
#pragma once

constexpr static unsigned int IMGFLAG_GRAY = 0x0001U;
constexpr static unsigned int IMGFLAG_FLIP = 0x0002U;

struct Image final {
    static void register_resource(void);

    stbi_uc* pixels;
    glm::ivec2 size;
};

#endif
