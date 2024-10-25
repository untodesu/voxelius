// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2024, Voxelius Contributors
#pragma once
#include <GLFW/glfw3.h>

struct GlfwKeyEvent final {
    int key {GLFW_KEY_UNKNOWN};
    int scancode {};
    int action {};
    int mods {};
};
