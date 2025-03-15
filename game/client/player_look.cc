#include "client/pch.hh"
#include "client/player_look.hh"

#include "core/angles.hh"
#include "core/config.hh"

#include "shared/dimension.hh"
#include "shared/head.hh"

#include "client/const.hh"
#include "client/gamepad_axis.hh"
#include "client/gamepad_button.hh"
#include "client/gamepad.hh"
#include "client/glfw.hh"
#include "client/globals.hh"
#include "client/keybind.hh"
#include "client/session.hh"
#include "client/settings.hh"

constexpr static float PITCH_MIN = -1.0f * cxpr::radians(90.0f);
constexpr static float PITCH_MAX = +1.0f * cxpr::radians(90.0f);

// Mouse options
static ConfigBoolean mouse_raw_input(true);
static ConfigUnsigned mouse_sensitivity(25U, 1U, 100U);

// Gamepad options
static ConfigFloat gamepad_fastlook_factor(1.5f, 1.0f, 5.0f);
static ConfigUnsigned gamepad_accel_pitch(15U, 1U, 100U);
static ConfigUnsigned gamepad_accel_yaw(25U, 1U, 100U);

// Gamepad axes
static ConfigGamepadAxis axis_pitch(GLFW_GAMEPAD_AXIS_LEFT_Y, false);
static ConfigGamepadAxis axis_yaw(GLFW_GAMEPAD_AXIS_LEFT_X, false);

// Gamepad buttons
static ConfigGamepadButton button_fastlook(GLFW_GAMEPAD_BUTTON_LEFT_THUMB);

static bool fastlook_enabled;
static glm::fvec2 last_cursor;

static void add_angles(float pitch, float yaw)
{
    if(session::is_ingame()) {
        auto &head = globals::dimension->entities.get<HeadComponent>(globals::player);

        head.angles[0] += pitch;
        head.angles[1] += yaw;
        head.angles[0] = cxpr::clamp(head.angles[0], PITCH_MIN, PITCH_MAX);
        head.angles = cxangles::wrap_180(head.angles);

        // Client-side head angles are not interpolated;
        // Re-assigning the previous state after the current
        // state has been already modified is certainly a way
        // to circumvent the interpolation applied to anything with a head
        globals::dimension->entities.emplace_or_replace<HeadComponentPrev>(globals::player, head);
    }
}

static void on_glfw_cursor_pos(const GlfwCursorPosEvent &event)
{
    if(gamepad::available && gamepad::active.get_value()) {
        // The player is assumed to be using
        // a gamepad instead of mouse and keyboard
        last_cursor = event.pos;
        return;
    }
    
    if(globals::gui_screen || !session::is_ingame()) {
        // UI is visible or we're not in-game
        last_cursor = event.pos;
        return;
    }

    auto dx = -0.01f * static_cast<float>(mouse_sensitivity.get_value()) * cxpr::radians(event.pos.x - last_cursor.x);
    auto dy = -0.01f * static_cast<float>(mouse_sensitivity.get_value()) * cxpr::radians(event.pos.y - last_cursor.y);
    add_angles(dy, dx);

    last_cursor = event.pos;
}

static void on_gamepad_button(const GamepadButtonEvent &event)
{
    if(button_fastlook.equals(event.button)) {
        if(event.action == GLFW_PRESS)
            fastlook_enabled = true;
        else fastlook_enabled = false;
    }
}

void player_look::init(void)
{
    globals::client_config.add_value("player_look.mouse.raw_input", mouse_raw_input);
    globals::client_config.add_value("player_look.mouse.sensitivity", mouse_sensitivity);
    globals::client_config.add_value("player_look.gamepad.fastlook_factor", gamepad_fastlook_factor);
    globals::client_config.add_value("player_look.gamepad.accel_pitch", gamepad_accel_pitch);
    globals::client_config.add_value("player_look.gamepad.accel_yaw", gamepad_accel_yaw);
    globals::client_config.add_value("player_look.gp_axis.pitch", axis_pitch);
    globals::client_config.add_value("player_look.gp_axis.yaw", axis_yaw);
    globals::client_config.add_value("player_look.gp_button.fastlook", button_fastlook);

    settings::add_slider(0, mouse_sensitivity, settings_location::MOUSE, "player_look.mouse.sensitivity", true);
    settings::add_checkbox(1, mouse_raw_input, settings_location::MOUSE, "player_look.mouse.raw_input", true);

    settings::add_slider(0, gamepad_accel_pitch, settings_location::GAMEPAD_GAMEPLAY, "player_look.gamepad.accel_pitch", false);
    settings::add_slider(1, gamepad_accel_yaw, settings_location::GAMEPAD_GAMEPLAY, "player_look.gamepad.accel_yaw", false);
    settings::add_gamepad_axis(2, axis_pitch, settings_location::GAMEPAD_GAMEPLAY, "player_look.gp_axis.pitch");
    settings::add_gamepad_axis(3, axis_yaw, settings_location::GAMEPAD_GAMEPLAY, "player_look.gp_axis.yaw");
    settings::add_slider(4, gamepad_fastlook_factor, settings_location::GAMEPAD_GAMEPLAY, "player_look.gamepad.fastlook_factor", true, "%.02f");
    settings::add_gamepad_button(5, button_fastlook, settings_location::GAMEPAD_GAMEPLAY, "player_look.gp_button.fastlook");

    fastlook_enabled = false;
    last_cursor.x = 0.5f * static_cast<float>(globals::width);
    last_cursor.y = 0.5f * static_cast<float>(globals::height);

    globals::dispatcher.sink<GlfwCursorPosEvent>().connect<&on_glfw_cursor_pos>();
    globals::dispatcher.sink<GamepadButtonEvent>().connect<&on_gamepad_button>();
}

void player_look::update_late(void)
{
    if(gamepad::available && gamepad::active.get_value() && !globals::gui_screen) {
        auto pitch_value = axis_pitch.get_value(gamepad::state, gamepad::deadzone.get_value());
        auto yaw_value = axis_yaw.get_value(gamepad::state, gamepad::deadzone.get_value());

        if(fastlook_enabled) {
            // Fastlook allows the camera to
            // rotate quicker when a button is held down
            pitch_value *= gamepad_fastlook_factor.get_value();
            yaw_value *= gamepad_fastlook_factor.get_value();
        }

        pitch_value *= 0.001f * static_cast<float>(gamepad_accel_pitch.get_value());
        yaw_value *= 0.001f * static_cast<float>(gamepad_accel_yaw.get_value());

        add_angles(pitch_value, yaw_value);
    }

    if(!globals::gui_screen && session::is_ingame) {
        glfwSetInputMode(globals::window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetInputMode(globals::window, GLFW_RAW_MOUSE_MOTION, mouse_raw_input.get_value());
    }
    else {
        glfwSetInputMode(globals::window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        glfwSetInputMode(globals::window, GLFW_RAW_MOUSE_MOTION, false);
    }
}
