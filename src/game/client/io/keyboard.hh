// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: keyboard.hh; Created: Tue Dec 30 2025 12:27:04
// Description: Keyboard handling

#ifndef CLIENT_IO_KEYBOARD_HH
#define CLIENT_IO_KEYBOARD_HH
#pragma once

class KeyEvent final {
public:
    constexpr explicit KeyEvent(int keycode, int scancode, int action, int modbits);

    constexpr int keycode(void) const noexcept;
    constexpr int scancode(void) const noexcept;
    constexpr int action(void) const noexcept;
    constexpr int modbits(void) const noexcept;

    constexpr bool is_keycode(int keycode) const noexcept;
    constexpr bool is_action(int action) const noexcept;
    constexpr bool has_modbits(int modbits) const noexcept;

    constexpr bool is_valid(void) const noexcept;

private:
    int m_keycode;
    int m_scancode;
    int m_action;
    int m_modbits;
};

namespace keyboard
{
void init(void);
} // namespace keyboard

constexpr KeyEvent::KeyEvent(int keycode, int scancode, int action, int modbits)
    : m_keycode(keycode), m_scancode(scancode), m_action(action), m_modbits(modbits)
{
    // empty
}

constexpr int KeyEvent::keycode(void) const noexcept
{
    return m_keycode;
}

constexpr int KeyEvent::scancode(void) const noexcept
{
    return m_scancode;
}

constexpr int KeyEvent::action(void) const noexcept
{
    return m_action;
}

constexpr int KeyEvent::modbits(void) const noexcept
{
    return m_modbits;
}

constexpr bool KeyEvent::is_keycode(int keycode) const noexcept
{
    return m_keycode == keycode;
}

constexpr bool KeyEvent::is_action(int action) const noexcept
{
    return m_action == action;
}

constexpr bool KeyEvent::has_modbits(int modbits) const noexcept
{
    return static_cast<bool>(m_modbits & modbits);
}

constexpr bool KeyEvent::is_valid(void) const noexcept
{
    return m_keycode >= 0 && m_keycode <= GLFW_KEY_LAST;
}

#endif
