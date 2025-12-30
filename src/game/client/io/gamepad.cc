// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: gamepad.cc
// Description: Gamepad support

#include "client/pch.hh"

#include "client/io/gamepad.hh"

#include "core/config/boolean.hh"
#include "core/config/number.hh"

#include "core/io/cmdline.hh"
#include "core/io/config_map.hh"
#include "core/io/physfs.hh"

#include "core/math/constexpr.hh"

#include "client/gui/settings.hh"

#include "client/globals.hh"
#include "client/toggles.hh"

constexpr static int INVALID_GAMEPAD_ID = INT_MAX;
constexpr static std::size_t NUM_AXES = static_cast<std::size_t>(GLFW_GAMEPAD_AXIS_LAST + 1);
constexpr static std::size_t NUM_BUTTONS = static_cast<std::size_t>(GLFW_GAMEPAD_BUTTON_LAST + 1);
constexpr static float GAMEPAD_AXIS_EVENT_THRESHOLD = 0.5f;

static int active_gamepad_id;

bool gamepad::available = false;
config::Float gamepad::deadzone(0.00f, 0.00f, 0.66f);
config::Boolean gamepad::active(false);
GLFWgamepadstate gamepad::state;
GLFWgamepadstate gamepad::last_state;

static void on_toggle_enable(const ToggleEnabledEvent& event)
{
    if(event.type == TOGGLE_USE_GAMEPAD) {
        gamepad::active.set_value(true);
        return;
    }
}

static void on_toggle_disable(const ToggleDisabledEvent& event)
{
    if(event.type == TOGGLE_USE_GAMEPAD) {
        gamepad::active.set_value(false);
        return;
    }
}

static void on_joystick_glfw(int joystick_id, int event_type)
{
    if((event_type == GLFW_CONNECTED) && glfwJoystickIsGamepad(joystick_id) && (active_gamepad_id == INVALID_GAMEPAD_ID)) {
        gamepad::available = true;

        active_gamepad_id = joystick_id;

        for(int i = 0; i < NUM_AXES; ++i) {
            gamepad::last_state.axes[i] = 0.0f;
        }

        for(int i = 0; i < NUM_BUTTONS; ++i) {
            gamepad::last_state.buttons[i] = GLFW_RELEASE;
        }

        spdlog::info("gamepad: detected gamepad: {}", glfwGetGamepadName(joystick_id));

        return;
    }

    if((event_type == GLFW_DISCONNECTED) && (active_gamepad_id == joystick_id)) {
        gamepad::available = false;

        active_gamepad_id = INVALID_GAMEPAD_ID;

        for(int i = 0; i < NUM_AXES; ++i) {
            gamepad::last_state.axes[i] = 0.0f;
        }

        for(int i = 0; i < NUM_BUTTONS; ++i) {
            gamepad::last_state.buttons[i] = GLFW_RELEASE;
        }

        spdlog::warn("gamepad: disconnected");

        return;
    }
}

void gamepad::init(void)
{
    gamepad::available = false;

    active_gamepad_id = INVALID_GAMEPAD_ID;

    globals::client_config.add_value("gamepad.deadzone", gamepad::deadzone);
    globals::client_config.add_value("gamepad.active", gamepad::active);

    settings::add_checkbox(0, gamepad::active, settings_location::GAMEPAD, "gamepad.active", true);
    settings::add_slider(1, gamepad::deadzone, settings_location::GAMEPAD, "gamepad.deadzone", true, "%.03f");

    std::string mappings_string;
    std::string_view mappings_path(cmdline::get("gpmap", "misc/gamecontrollerdb.txt"));

    if(physfs::read_file(mappings_path, mappings_string)) {
        spdlog::info("gamepad: using mappings from {}", mappings_path);
        glfwUpdateGamepadMappings(mappings_string.c_str());
    }

    for(int joystick = 0; joystick <= GLFW_JOYSTICK_LAST; joystick += 1) {
        if(glfwJoystickIsGamepad(joystick)) {
            gamepad::available = true;

            active_gamepad_id = joystick;

            for(int i = 0; i < NUM_AXES; gamepad::last_state.axes[i++] = 0.0f) {
                // empty
            }

            for(int i = 0; i < NUM_BUTTONS; gamepad::last_state.buttons[i++] = GLFW_RELEASE) {
                // empty
            }

            spdlog::info("gamepad: detected gamepad: {}", glfwGetGamepadName(joystick));

            break;
        }
    }

    for(int i = 0; i < NUM_AXES; gamepad::state.axes[i++] = 0.0f) {
        // empty
    }

    for(int i = 0; i < NUM_BUTTONS; gamepad::state.buttons[i++] = GLFW_RELEASE) {
        // empty
    }

    globals::dispatcher.sink<ToggleEnabledEvent>().connect<&on_toggle_enable>();
    globals::dispatcher.sink<ToggleDisabledEvent>().connect<&on_toggle_disable>();

    spdlog::info("gamepad: taking over device callbacks");
    glfwSetJoystickCallback(&on_joystick_glfw);
}

void gamepad::update_late(void)
{
    if(active_gamepad_id == INVALID_GAMEPAD_ID) {
        // No active gamepad found
        return;
    }

    if(glfwGetGamepadState(active_gamepad_id, &gamepad::state)) {
        for(int i = 0; i < NUM_AXES; ++i) {
            auto is_press = true;
            is_press = is_press && glm::abs(gamepad::state.axes[i] > GAMEPAD_AXIS_EVENT_THRESHOLD);
            is_press = is_press && glm::abs(gamepad::last_state.axes[i] <= GAMEPAD_AXIS_EVENT_THRESHOLD);

            if(is_press) {
                globals::dispatcher.enqueue(GamepadAxisEvent(i, GLFW_PRESS));
                continue;
            }

            auto is_release = true;
            is_release = is_release && glm::abs(gamepad::state.axes[i]) <= GAMEPAD_AXIS_EVENT_THRESHOLD;
            is_release = is_release && glm::abs(gamepad::last_state.axes[i]) > GAMEPAD_AXIS_EVENT_THRESHOLD;

            if(is_release) {
                globals::dispatcher.enqueue(GamepadAxisEvent(i, GLFW_RELEASE));
                continue;
            }
        }

        for(int i = 0; i < NUM_BUTTONS; ++i) {
            if(gamepad::state.buttons[i] == gamepad::last_state.buttons[i]) {
                // Nothing happens
                continue;
            }

            globals::dispatcher.enqueue(GamepadButtonEvent(i, gamepad::state.buttons[i]));
        }
    }

    gamepad::last_state = gamepad::state;
}
