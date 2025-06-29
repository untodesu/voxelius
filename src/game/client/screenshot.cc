#include "client/pch.hh"

#include "client/screenshot.hh"

#include "core/config.hh"
#include "core/epoch.hh"

#include "client/chat.hh"
#include "client/glfw.hh"
#include "client/globals.hh"
#include "client/keybind.hh"
#include "client/language.hh"
#include "client/settings.hh"
#include "client/toggles.hh"

static ConfigKeyBind screenshot_key(GLFW_KEY_F2);

static void stbi_png_physfs_callback(void* context, void* data, int size)
{
    PHYSFS_writeBytes(reinterpret_cast<PHYSFS_File*>(context), data, size);
}

static void on_glfw_key(const GlfwKeyEvent& event)
{
    if(!globals::gui_keybind_ptr && !toggles::is_sequence_await) {
        if(screenshot_key.equals(event.key) && (event.action == GLFW_PRESS)) {
            screenshot::take();
            return;
        }
    }
}

void screenshot::init(void)
{
    globals::client_config.add_value("screenshot.key", screenshot_key);

    settings::add_keybind(0, screenshot_key, settings_location::KEYBOARD_MISC, "key.screenshot");

    globals::dispatcher.sink<GlfwKeyEvent>().connect<&on_glfw_key>();
}

void screenshot::take(void)
{
    auto stride = 3 * globals::width;
    auto length = 3 * globals::width * globals::height;
    auto pixels = new std::byte[length];

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    GLint old_pack_alignment;
    glGetIntegerv(GL_PACK_ALIGNMENT, &old_pack_alignment);

    // The window can be of any size, including irregular
    // values such as, say 641x480, while there is a default
    // alignment value of sorts that might result in a corrupted
    // image; we set GL_PACK_ALIGNMENT to 1, enabling byte-alignment
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    glReadPixels(0, 0, globals::width, globals::height, GL_RGB, GL_UNSIGNED_BYTE, pixels);

    // Restore the old pack alignment value
    glPixelStorei(GL_PACK_ALIGNMENT, old_pack_alignment);

    const auto directory = std::string("screenshots");
    const auto filename = std::format("{}.png", epoch::microseconds());
    const auto filepath = std::format("{}/{}", directory, filename);

    PHYSFS_mkdir(directory.c_str());

    if(auto file = PHYSFS_openWrite(filepath.c_str())) {
        stbi_flip_vertically_on_write(true);
        stbi_write_png_to_func(&stbi_png_physfs_callback, file, globals::width, globals::height, 3, pixels, stride);

        spdlog::info("screenshot: wrote {}", filepath);

        client_chat::print(std::format("{} {}", language::resolve("chat.screenshot_message"), filename));

        PHYSFS_close(file);
    }

    delete[] pixels;
}
