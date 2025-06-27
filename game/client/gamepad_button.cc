#include "client/pch.hh"

#include "client/gamepad_button.hh"

#include "core/constexpr.hh"

#include "client/gamepad.hh"

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

ConfigGamepadButton::ConfigGamepadButton(void)
{
    m_gamepad_button = INVALID_GAMEPAD_BUTTON;
    m_name = UNKNOWN_BUTTON_NAME;
}

ConfigGamepadButton::ConfigGamepadButton(int button)
{
    m_gamepad_button = button;
    m_name = get_button_name(button);
}

const char* ConfigGamepadButton::get(void) const
{
    return m_name;
}

void ConfigGamepadButton::set(const char* value)
{
    for(const auto& it : button_names) {
        if(!std::strcmp(it.second, value)) {
            m_gamepad_button = it.first;
            m_name = it.second;
            return;
        }
    }

    m_gamepad_button = INVALID_GAMEPAD_BUTTON;
    m_name = UNKNOWN_BUTTON_NAME;
}

int ConfigGamepadButton::get_button(void) const
{
    return m_gamepad_button;
}

void ConfigGamepadButton::set_button(int button)
{
    m_gamepad_button = button;
    m_name = get_button_name(button);
}

bool ConfigGamepadButton::equals(int button) const
{
    return m_gamepad_button == button;
}

bool ConfigGamepadButton::is_pressed(const GLFWgamepadstate& state) const
{
    return m_gamepad_button < vx::array_size(state.buttons) && state.buttons[m_gamepad_button] == GLFW_PRESS;
}
