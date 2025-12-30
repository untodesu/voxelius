// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: mouse.hh; Created: Tue Dec 30 2025 12:33:48
// Description: Mouse and scroll wheel handling

#ifndef CLIENT_IO_MOUSE_HH
#define CLIENT_IO_MOUSE_HH
#pragma once

class CursorPosEvent final {
public:
    constexpr explicit CursorPosEvent(double xpos, double ypos);

    constexpr float xpos(void) const noexcept;
    constexpr float ypos(void) const noexcept;
    constexpr const glm::fvec2& position(void) const noexcept;

private:
    glm::fvec2 m_position;
};

class MouseButtonEvent final {
public:
    constexpr explicit MouseButtonEvent(int button, int action, int modbits);

    constexpr int button(void) const noexcept;
    constexpr int action(void) const noexcept;
    constexpr int modbits(void) const noexcept;

    constexpr bool is_button(int button) const noexcept;
    constexpr bool is_action(int action) const noexcept;
    constexpr bool has_modbits(int modbits) const noexcept;

    constexpr bool is_valid(void) const noexcept;

private:
    int m_button;
    int m_action;
    int m_modbits;
};

class ScrollEvent final {
public:
    constexpr explicit ScrollEvent(double xoffset, double yoffset);

    constexpr float xoffset(void) const noexcept;
    constexpr float yoffset(void) const noexcept;
    constexpr const glm::fvec2& offset(void) const noexcept;

private:
    glm::fvec2 m_offset;
};

namespace mouse
{
void init(void);
} // namespace mouse

constexpr CursorPosEvent::CursorPosEvent(double xpos, double ypos)
{
    m_position.x = static_cast<float>(xpos);
    m_position.y = static_cast<float>(ypos);
}

constexpr float CursorPosEvent::xpos(void) const noexcept
{
    return m_position.x;
}

constexpr float CursorPosEvent::ypos(void) const noexcept
{
    return m_position.y;
}

constexpr const glm::fvec2& CursorPosEvent::position(void) const noexcept
{
    return m_position;
}

constexpr MouseButtonEvent::MouseButtonEvent(int button, int action, int modbits) : m_button(button), m_action(action), m_modbits(modbits)
{
    // empty
}

constexpr int MouseButtonEvent::button(void) const noexcept
{
    return m_button;
}

constexpr int MouseButtonEvent::action(void) const noexcept
{
    return m_action;
}

constexpr int MouseButtonEvent::modbits(void) const noexcept
{
    return m_modbits;
}

constexpr bool MouseButtonEvent::is_button(int button) const noexcept
{
    return m_button == button;
}

constexpr bool MouseButtonEvent::is_action(int action) const noexcept
{
    return m_action == action;
}

constexpr bool MouseButtonEvent::has_modbits(int modbits) const noexcept
{
    return static_cast<bool>(m_modbits & modbits);
}

constexpr bool MouseButtonEvent::is_valid(void) const noexcept
{
    return m_button >= 0 && m_button <= GLFW_MOUSE_BUTTON_LAST;
}

constexpr ScrollEvent::ScrollEvent(double xoffset, double yoffset)
{
    m_offset.x = static_cast<float>(xoffset);
    m_offset.y = static_cast<float>(yoffset);
}

constexpr float ScrollEvent::xoffset(void) const noexcept
{
    return m_offset.x;
}

constexpr float ScrollEvent::yoffset(void) const noexcept
{
    return m_offset.y;
}

constexpr const glm::fvec2& ScrollEvent::offset(void) const noexcept
{
    return m_offset;
}

#endif
