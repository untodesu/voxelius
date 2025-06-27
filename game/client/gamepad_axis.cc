#include "client/pch.hh"

#include "client/gamepad_axis.hh"

#include "core/constexpr.hh"

#include "client/gamepad.hh"

constexpr static const char* UNKNOWN_AXIS_NAME = "UNKNOWN";

static const std::pair<int, const char*> axis_names[] = {
    { GLFW_GAMEPAD_AXIS_LEFT_X, "LEFT_X" },
    { GLFW_GAMEPAD_AXIS_LEFT_Y, "LEFT_Y" },
    { GLFW_GAMEPAD_AXIS_RIGHT_X, "RIGHT_X" },
    { GLFW_GAMEPAD_AXIS_RIGHT_Y, "RIGHT_Y" },
    { GLFW_GAMEPAD_AXIS_LEFT_TRIGGER, "LEFT_TRIG" },
    { GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER, "RIGHT_TRIG" },
};

static const char* get_axis_name(int axis)
{
    for(const auto& it : axis_names) {
        if(it.first != axis) {
            continue;
        }

        return it.second;
    }

    return UNKNOWN_AXIS_NAME;
}

ConfigGamepadAxis::ConfigGamepadAxis(void) : ConfigGamepadAxis(INVALID_GAMEPAD_AXIS, false)
{
}

ConfigGamepadAxis::ConfigGamepadAxis(int axis, bool inverted)
{
    m_inverted = inverted;
    m_gamepad_axis = axis;
    m_name = get_axis_name(axis);
    m_full_string = std::format("{}:{}", m_name, m_inverted ? 1U : 0U);
}

const char* ConfigGamepadAxis::get(void) const
{
    return m_full_string.c_str();
}

void ConfigGamepadAxis::set(const char* value)
{
    char new_name[64];
    unsigned int new_invert;

    if(2 == std::sscanf(value, "%63[^:]:%u", new_name, &new_invert)) {
        for(const auto& it : axis_names) {
            if(!std::strcmp(it.second, new_name)) {
                m_inverted = new_invert;
                m_gamepad_axis = it.first;
                m_name = get_axis_name(m_gamepad_axis);
                m_full_string = std::format("{}:{}", m_name, m_inverted ? 1U : 0U);
                return;
            }
        }
    }

    m_inverted = false;
    m_gamepad_axis = INVALID_GAMEPAD_AXIS;
    m_name = UNKNOWN_AXIS_NAME;
    m_full_string = std::format("{}:{}", m_name, m_inverted ? 1U : 0U);
}

int ConfigGamepadAxis::get_axis(void) const
{
    return m_gamepad_axis;
}

void ConfigGamepadAxis::set_axis(int axis)
{
    m_gamepad_axis = axis;
    m_name = get_axis_name(axis);
    m_full_string = std::format("{}:{}", m_name, m_inverted ? 1U : 0U);
}

bool ConfigGamepadAxis::is_inverted(void) const
{
    return m_inverted;
}

void ConfigGamepadAxis::set_inverted(bool inverted)
{
    m_inverted = inverted;
    m_full_string = std::format("{}:{}", m_name, m_inverted ? 1U : 0U);
}

float ConfigGamepadAxis::get_value(const GLFWgamepadstate& state, float deadzone) const
{
    if(m_gamepad_axis <= vx::array_size(state.axes)) {
        auto value = state.axes[m_gamepad_axis];

        if(vx::abs(value) > deadzone) {
            return m_inverted ? -value : value;
        }

        return 0.0f;
    }

    return 0.0f;
}

const char* ConfigGamepadAxis::get_name(void) const
{
    return m_name;
}
