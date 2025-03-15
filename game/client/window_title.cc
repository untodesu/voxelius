#include "client/pch.hh"
#include "client/window_title.hh"

#include "core/feature.hh"
#include "core/version.hh"

#include "shared/splash.hh"

#include "client/globals.hh"

void window_title::update(void)
{
    auto string = fmt::format("Voxelius {}: {}", PROJECT_VERSION_STRING, splash::get());

    glfwSetWindowTitle(globals::window, string.c_str());
}
