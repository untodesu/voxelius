#include "client/pch.hh"
#include "client/window_title.hh"

#include "core/feature.hh"
#include "core/version.hh"

#include "shared/splash.hh"

#include "client/globals.hh"

void window_title::update(void)
{
    std::string title;

    if(globals::sound_ctx && globals::sound_dev)
        title = fmt::format("Voxelius {}: {}", PROJECT_VERSION_STRING, splash::get());
    else title = fmt::format("Voxelius {}: {} [NOSOUND]", PROJECT_VERSION_STRING, splash::get());

    glfwSetWindowTitle(globals::window, title.c_str());
}
