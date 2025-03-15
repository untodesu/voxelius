#include "client/pch.hh"
#include "client/toggles.hh"

#include "core/config.hh"

#include "client/chat.hh"
#include "client/const.hh"
#include "client/gamepad.hh"
#include "client/glfw.hh"
#include "client/globals.hh"
#include "client/language.hh"

bool toggles::is_sequence_await = false;

bool toggles::draw_chunk_borders = false;
bool toggles::render_fullbright = false;
bool toggles::render_wireframe = false;

#if defined(NDEBUG)
bool toggles::draw_metrics = false;
#else
bool toggles::draw_metrics = true;
#endif

static void toggle_value(bool *value, const char *description)
{
    value[0] = !value[0];

    if(description) {
        if(value[0])
            client_chat::print(fmt::format("[debug] {} ON", description));
        else client_chat::print(fmt::format("[debug] {} OFF", description));
    }
}

static void toggle_value_config(ConfigBoolean &value, const char *description)
{
    value.set_value(!value.get_value());

    if(description) {
        if(value.get_value())
            client_chat::print(fmt::format("[debug] {} ON", description));
        else client_chat::print(fmt::format("[debug] {} OFF", description));
    }
}

static void on_glfw_key(const GlfwKeyEvent &event)
{
    if(globals::gui_keybind_ptr) {
        // The UI keybind subsystem has the authority
        // over debug toggles and it hogs the input keys
        return;
    }

    if(event.key == DEBUG_KEY) {
        if(event.action == GLFW_PRESS) {
            toggles::is_sequence_await = true;
            return;
        }

        if(event.action == GLFW_RELEASE) {
            toggles::is_sequence_await = false;
            return;
        }
    }

    if((event.action == GLFW_PRESS) && toggles::is_sequence_await) {
        switch(event.key) {
            case GLFW_KEY_G:
                toggle_value(&toggles::draw_chunk_borders, "chunk borders");
                return;
            case GLFW_KEY_V:
                toggle_value(&toggles::draw_metrics, nullptr);
                return;
            case GLFW_KEY_J:
                toggle_value(&toggles::render_fullbright, nullptr);
                return;
            case GLFW_KEY_Z:
                toggle_value(&toggles::render_wireframe, "wireframe");
                return;
            case GLFW_KEY_P:
                toggle_value_config(gamepad::active, "gamepad input");
                return;
            case GLFW_KEY_L:
                // This causes the language subsystem
                // to re-parse the JSON file essentially
                // causing the game to soft-reload language
                language::set(language::get_current());
                return;
        }
    }
}

void toggles::init(void)
{
    globals::dispatcher.sink<GlfwKeyEvent>().connect<&on_glfw_key>();
}
