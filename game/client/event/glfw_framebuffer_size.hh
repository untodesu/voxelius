// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2024, Voxelius Contributors
#pragma once

struct GlfwFramebufferSizeEvent final {
    int width {};
    int height {};
    float aspect {};
};
