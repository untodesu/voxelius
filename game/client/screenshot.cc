// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2024, Voxelius Contributors
#include <common/config.hh>
#include <common/epoch.hh>
#include <common/fstools.hh>
#include <entt/signal/dispatcher.hpp>
#include <game/client/event/glfw_key.hh>
#include <game/client/globals.hh>
#include <game/client/screenshot.hh>
#include <game/client/settings.hh>
#include <game/client/toggles.hh>
#include <spdlog/fmt/fmt.h>
#include <stb_image_write.h>

static int key_screenshot = GLFW_KEY_F2;

static void png_write(void *context, void *data, int size)
{
    PHYSFS_File *file = reinterpret_cast<PHYSFS_File *>(context);
    PHYSFS_writeBytes(file, data, size);
}

static void on_glfw_key(const GlfwKeyEvent &event)
{
    if(!globals::gui_keybind_ptr && !toggles::is_sequence_await) {    
        if((event.key == key_screenshot) && (event.action == GLFW_PRESS)) {
            screenshot::take();
            return;
        }
    }
}

void screenshot::init(void)
{
    Config::add(globals::client_config, "screenshot.key", key_screenshot);
    
    settings::add_key_binding(0, settings::KEYBOARD_MISC, "key.screenshot", key_screenshot);

    globals::dispatcher.sink<GlfwKeyEvent>().connect<&on_glfw_key>();
}

void screenshot::take(void)
{
    const std::size_t stride = 3 * globals::width;
    const std::size_t length = 3 * globals::width * globals::height;
    std::uint8_t *pixels = new std::uint8_t[length];

    // We'll read pixel data from the window's main
    // framebuffer; we want everything including GUI
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    GLint old_pack_alignment;
    glGetIntegerv(GL_PACK_ALIGNMENT, &old_pack_alignment);

    // The window can be of any size, including irregular
    // values such as, say 641x480, while there is a default
    // alignment value of sorts that might result in a corrupted
    // image (https://github.com/voxelius/voxelius/issues/29#issuecomment-2438432281)
    // We set GL_PACK_ALIGNMENT to 1, enabling byte-alignment
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    glReadPixels(0, 0, globals::width, globals::height, GL_RGB, GL_UNSIGNED_BYTE, pixels);

    // Restore the old pack alignment value
    glPixelStorei(GL_PACK_ALIGNMENT, old_pack_alignment);

    const std::string dirname = std::string("screenshots");
    const std::string path = fmt::format("{}/{}.png", dirname, epoch::microseconds());

    PHYSFS_mkdir(dirname.c_str());

    if(PHYSFS_File *file = PHYSFS_openWrite(path.c_str())) {
        stbi_flip_vertically_on_write(true);
        stbi_write_png_to_func(&png_write, file, globals::width, globals::height, 3, pixels, 3 * globals::width);
        PHYSFS_close(file);
    }

    delete[] pixels;
}
