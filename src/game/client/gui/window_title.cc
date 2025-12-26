// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: window_title.cc
// Description: Random MOTD in the window title

#include "client/pch.hh"

#include "client/gui/window_title.hh"

#include "core/version.hh"

#include "shared/splash.hh"

#include "client/globals.hh"

void window_title::update(void)
{
    glfwSetWindowTitle(globals::window, std::format("Voxelius {}: {}", version::triplet, splash::get()).c_str());
}
