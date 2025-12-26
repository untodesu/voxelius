#include "client/pch.hh"

#include "client/gui/window_title.hh"

#include "core/version.hh"

#include "shared/splash.hh"

#include "client/globals.hh"

void window_title::update(void)
{
    glfwSetWindowTitle(globals::window, std::format("Voxelius {}: {}", version::triplet, splash::get()).c_str());
}
