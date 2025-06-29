#include "client/pch.hh"

#include "client/window_title.hh"

#include "core/version.hh"

#include "shared/splash.hh"

#include "client/globals.hh"

void window_title::update(void)
{
    std::string title;

    if(globals::sound_ctx && globals::sound_dev) {
        title = std::format("Voxelius {}: {}", project_version_string, splash::get());
    } else {
        title = std::format("Voxelius {}: {} [NOSOUND]", project_version_string, splash::get());
    }

    glfwSetWindowTitle(globals::window, title.c_str());
}
