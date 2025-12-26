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
#include "core/math/constexpr.hh"

#include "client/gui/settings.hh"
#include "client/io/glfw.hh"

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

static void on_glfw_joystick_event(const GlfwJoystickEvent& event)
{
    if((event.event_type == GLFW_CONNECTED) && glfwJoystickIsGamepad(event.joystick_id) && (active_gamepad_id == INVALID_GAMEPAD_ID)) {
        gamepad::available = true;

        active_gamepad_id = event.joystick_id;

        for(int i = 0; i < NUM_AXES; gamepad::last_state.axes[i++] = 0.0f) {
            // empty
        }

        for(int i = 0; i < NUM_BUTTONS; gamepad::last_state.buttons[i++] = GLFW_RELEASE) {
            // empty
        }

        spdlog::info("gamepad: detected gamepad: {}", glfwGetGamepadName(event.joystick_id));

        return;
    }

    if((event.event_type == GLFW_DISCONNECTED) && (active_gamepad_id == event.joystick_id)) {
        gamepad::available = false;

        active_gamepad_id = INVALID_GAMEPAD_ID;

        for(int i = 0; i < NUM_AXES; gamepad::last_state.axes[i++] = 0.0f) {
            // empty
        }

        for(int i = 0; i < NUM_BUTTONS; gamepad::last_state.buttons[i++] = GLFW_RELEASE) {
            // empty
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

    auto mappings_path = cmdline::get_cstr("gpmap", "misc/gamecontrollerdb.txt");
    auto mappings_file = PHYSFS_openRead(mappings_path);

    if(mappings_file) {
        spdlog::info("gamepad: using mappings from {}", mappings_path);
        auto mappings_string = std::string(PHYSFS_fileLength(mappings_file), char(0x00));
        PHYSFS_readBytes(mappings_file, mappings_string.data(), mappings_string.size());
        glfwUpdateGamepadMappings(mappings_string.c_str());
        PHYSFS_close(mappings_file);
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
    globals::dispatcher.sink<GlfwJoystickEvent>().connect<&on_glfw_joystick_event>();
}

void gamepad::update_late(void)
{
    if(active_gamepad_id == INVALID_GAMEPAD_ID) {
        // No active gamepad found
        return;
    }

    if(glfwGetGamepadState(active_gamepad_id, &gamepad::state)) {
        for(int i = 0; i < NUM_AXES; ++i) {
            if((glm::abs(gamepad::state.axes[i]) > GAMEPAD_AXIS_EVENT_THRESHOLD)
                && (glm::abs(gamepad::last_state.axes[i]) <= GAMEPAD_AXIS_EVENT_THRESHOLD)) {
                GamepadAxisEvent event;
                event.action = GLFW_PRESS;
                event.axis = i;
                globals::dispatcher.enqueue(event);
                continue;
            }

            if((glm::abs(gamepad::state.axes[i]) <= GAMEPAD_AXIS_EVENT_THRESHOLD)
                && (glm::abs(gamepad::last_state.axes[i]) > GAMEPAD_AXIS_EVENT_THRESHOLD)) {
                GamepadAxisEvent event;
                event.action = GLFW_RELEASE;
                event.axis = i;
                globals::dispatcher.enqueue(event);
                continue;
            }
        }

        for(int i = 0; i < NUM_BUTTONS; ++i) {
            if(gamepad::state.buttons[i] == gamepad::last_state.buttons[i]) {
                // Nothing happens
                continue;
            }

            GamepadButtonEvent event;
            event.action = gamepad::state.buttons[i];
            event.button = i;
            globals::dispatcher.enqueue(event);
        }
    }

    gamepad::last_state = gamepad::state;
}
