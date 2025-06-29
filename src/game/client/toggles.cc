#include "client/pch.hh"

#include "client/toggles.hh"

#include "core/config.hh"

#include "client/chat.hh"
#include "client/const.hh"
#include "client/gamepad.hh"
#include "client/glfw.hh"
#include "client/globals.hh"
#include "client/language.hh"

struct ToggleInfo final {
    const char* description;
    int glfw_keycode;
    bool is_enabled;
};

bool toggles::is_sequence_await = false;

static ToggleInfo toggle_infos[TOGGLE_COUNT];

static void print_toggle_state(const ToggleInfo& info)
{
    if(info.description) {
        if(info.is_enabled) {
            client_chat::print(std::format("[toggles] {} ON", info.description));
        } else {
            client_chat::print(std::format("[toggles] {} OFF", info.description));
        }
    }
}

static void toggle_value(ToggleInfo& info, toggle_type type)
{
    if(info.is_enabled) {
        info.is_enabled = false;
        globals::dispatcher.trigger(ToggleDisabledEvent { type });
    } else {
        info.is_enabled = true;
        globals::dispatcher.trigger(ToggleEnabledEvent { type });
    }

    print_toggle_state(info);
}

static void on_glfw_key(const GlfwKeyEvent& event)
{
    if(globals::gui_keybind_ptr) {
        // The UI keybind subsystem has the authority
        // over debug toggles and it hogs the input keys
        return;
    }

    if(event.key == DEBUG_KEY) {
        if(event.action == GLFW_PRESS) {
            toggles::is_sequence_await = true;
            ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;
            return;
        }

        if(event.action == GLFW_RELEASE) {
            toggles::is_sequence_await = false;
            ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
            return;
        }
    }

    if((event.action == GLFW_PRESS) && toggles::is_sequence_await) {
        if(event.key == GLFW_KEY_L) {
            // This causes the language subsystem
            // to re-parse the JSON file essentially
            // causing the game to soft-reload language
            language::set(language::get_current());
            return;
        }

        for(toggle_type i = 0; i < TOGGLE_COUNT; ++i) {
            if(event.key == toggle_infos[i].glfw_keycode) {
                toggle_value(toggle_infos[i], i);
                return;
            }
        }
    }
}

void toggles::init(void)
{
    toggle_infos[TOGGLE_WIREFRAME].description = "wireframe";
    toggle_infos[TOGGLE_WIREFRAME].glfw_keycode = GLFW_KEY_Z;
    toggle_infos[TOGGLE_WIREFRAME].is_enabled = false;

    toggle_infos[TOGGLE_FULLBRIGHT].description = "fullbright";
    toggle_infos[TOGGLE_FULLBRIGHT].glfw_keycode = GLFW_KEY_J;
    toggle_infos[TOGGLE_FULLBRIGHT].is_enabled = false;

    toggle_infos[TOGGLE_CHUNK_AABB].description = "chunk Borders";
    toggle_infos[TOGGLE_CHUNK_AABB].glfw_keycode = GLFW_KEY_G;
    toggle_infos[TOGGLE_CHUNK_AABB].is_enabled = false;

    toggle_infos[TOGGLE_METRICS_UI].description = nullptr;
    toggle_infos[TOGGLE_METRICS_UI].glfw_keycode = GLFW_KEY_V;
    toggle_infos[TOGGLE_METRICS_UI].is_enabled = false;

    toggle_infos[TOGGLE_USE_GAMEPAD].description = "gamepad input";
    toggle_infos[TOGGLE_USE_GAMEPAD].glfw_keycode = GLFW_KEY_P;
    toggle_infos[TOGGLE_USE_GAMEPAD].is_enabled = false;

    toggle_infos[TOGGLE_PM_FLIGHT].description = "flight mode";
    toggle_infos[TOGGLE_PM_FLIGHT].glfw_keycode = GLFW_KEY_F;
    toggle_infos[TOGGLE_PM_FLIGHT].is_enabled = false;

#ifndef NDEBUG
    toggle_infos[TOGGLE_WIREFRAME].is_enabled = true;
#endif

    globals::dispatcher.sink<GlfwKeyEvent>().connect<&on_glfw_key>();
}

void toggles::init_late(void)
{
    for(toggle_type i = 0; i < TOGGLE_COUNT; ++i) {
        if(toggle_infos[i].is_enabled) {
            globals::dispatcher.trigger(ToggleEnabledEvent { i });
        } else {
            globals::dispatcher.trigger(ToggleDisabledEvent { i });
        }
    }
}

bool toggles::get(toggle_type type)
{
    if(type < TOGGLE_COUNT) {
        return toggle_infos[type].is_enabled;
    } else {
        return false;
    }
}

void toggles::set(toggle_type type, bool value)
{
    if(type < TOGGLE_COUNT) {
        if(value) {
            toggle_infos[type].is_enabled = true;
            globals::dispatcher.trigger(ToggleEnabledEvent { type });
        } else {
            toggle_infos[type].is_enabled = false;
            globals::dispatcher.trigger(ToggleDisabledEvent { type });
        }

        print_toggle_state(toggle_infos[type]);
    }
}
