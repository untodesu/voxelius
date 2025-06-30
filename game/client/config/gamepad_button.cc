#include "client/pch.hh"

#include "client/config/gamepad_button.hh"

#include "core/math/constexpr.hh"

#include "client/io/gamepad.hh"

constexpr static const char* UNKNOWN_BUTTON_NAME = "UNKNOWN";

static const std::pair<int, const char*> button_names[] = {
    { GLFW_GAMEPAD_BUTTON_A, "A" },
    { GLFW_GAMEPAD_BUTTON_B, "B" },
    { GLFW_GAMEPAD_BUTTON_X, "X" },
    { GLFW_GAMEPAD_BUTTON_Y, "Y" },
    { GLFW_GAMEPAD_BUTTON_LEFT_BUMPER, "L_BUMP" },
    { GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER, "R_BUMP" },
    { GLFW_GAMEPAD_BUTTON_BACK, "BACK" },
    { GLFW_GAMEPAD_BUTTON_START, "START" },
    { GLFW_GAMEPAD_BUTTON_GUIDE, "GUIDE" },
    { GLFW_GAMEPAD_BUTTON_LEFT_THUMB, "L_THUMB" },
    { GLFW_GAMEPAD_BUTTON_RIGHT_THUMB, "R_THUMB" },
    { GLFW_GAMEPAD_BUTTON_DPAD_UP, "DPAD_UP" },
    { GLFW_GAMEPAD_BUTTON_DPAD_RIGHT, "DPAD_RIGHT" },
    { GLFW_GAMEPAD_BUTTON_DPAD_DOWN, "DPAD_DOWN" },
    { GLFW_GAMEPAD_BUTTON_DPAD_LEFT, "DPAD_LEFT" },
};

static const char* get_button_name(int button)
{
    for(const auto& it : button_names) {
        if(it.first == button) {
            return it.second;
        }
    }

    return UNKNOWN_BUTTON_NAME;
}

config::GamepadButton::GamepadButton(void)
{
    m_gamepad_button = io::INVALID_GAMEPAD_BUTTON;
    m_name = UNKNOWN_BUTTON_NAME;
}

config::GamepadButton::GamepadButton(int button)
{
    m_gamepad_button = button;
    m_name = get_button_name(button);
}

const char* config::GamepadButton::get(void) const
{
    return m_name;
}

void config::GamepadButton::set(const char* value)
{
    for(const auto& it : button_names) {
        if(!std::strcmp(it.second, value)) {
            m_gamepad_button = it.first;
            m_name = it.second;
            return;
        }
    }

    m_gamepad_button = io::INVALID_GAMEPAD_BUTTON;
    m_name = UNKNOWN_BUTTON_NAME;
}

int config::GamepadButton::get_button(void) const
{
    return m_gamepad_button;
}

void config::GamepadButton::set_button(int button)
{
    m_gamepad_button = button;
    m_name = get_button_name(button);
}

bool config::GamepadButton::equals(int button) const
{
    return m_gamepad_button == button;
}

bool config::GamepadButton::is_pressed(const GLFWgamepadstate& state) const
{
    return m_gamepad_button < math::array_size(state.buttons) && state.buttons[m_gamepad_button] == GLFW_PRESS;
}
