// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: gamepad.hh
// Description: Gamepad support

#ifndef CLIENT_IO_GAMEPAD_HH
#define CLIENT_IO_GAMEPAD_HH
#pragma once

constexpr static int INVALID_GAMEPAD_AXIS = INT_MAX;
constexpr static int INVALID_GAMEPAD_BUTTON = INT_MAX;

// This simulates buttons using axes. When an axis
// value exceeds 1.5 times the deadzone, the event is
// queued with a GLFW_PRESS action, when it falls back
// below the threshold, the event is queued with GLFW_RELEASE action
class GamepadAxisEvent final {
public:
    constexpr explicit GamepadAxisEvent(int axis, int action);

    constexpr int axis(void) const noexcept;
    constexpr int action(void) const noexcept;

    constexpr bool is_axis(int axis) const noexcept;
    constexpr bool is_action(int action) const noexcept;

private:
    int m_axis;
    int m_action;
};

// This smears GLFW event sugar over gamepad polling
// system. Whenever it detects a state change, the event
// is queued with an appropriate action
class GamepadButtonEvent final {
public:
    constexpr explicit GamepadButtonEvent(int button, int action);

    constexpr int button(void) const noexcept;
    constexpr int action(void) const noexcept;

    constexpr bool is_button(int button) const noexcept;
    constexpr bool is_action(int action) const noexcept;

private:
    int m_action;
    int m_button;
};

namespace config
{
class Boolean;
class Float;
} // namespace config

struct GLFWgamepadstate;

namespace gamepad
{
extern bool available;
extern config::Float deadzone;
extern config::Boolean active;
extern GLFWgamepadstate state;
extern GLFWgamepadstate last_state;
} // namespace gamepad

namespace gamepad
{
void init(void);
void update_late(void);
} // namespace gamepad

constexpr GamepadAxisEvent::GamepadAxisEvent(int axis, int action) : m_axis(axis), m_action(action)
{
    // empty
}

constexpr int GamepadAxisEvent::axis(void) const noexcept
{
    return m_axis;
}

constexpr int GamepadAxisEvent::action(void) const noexcept
{
    return m_action;
}

constexpr bool GamepadAxisEvent::is_axis(int axis) const noexcept
{
    return m_axis == axis;
}

constexpr bool GamepadAxisEvent::is_action(int action) const noexcept
{
    return m_action == action;
}

constexpr GamepadButtonEvent::GamepadButtonEvent(int button, int action) : m_button(button), m_action(action)
{
    // empty
}

constexpr int GamepadButtonEvent::button(void) const noexcept
{
    return m_button;
}

constexpr int GamepadButtonEvent::action(void) const noexcept
{
    return m_action;
}

constexpr bool GamepadButtonEvent::is_button(int button) const noexcept
{
    return m_button == button;
}

constexpr bool GamepadButtonEvent::is_action(int action) const noexcept
{
    return m_action == action;
}

#endif
