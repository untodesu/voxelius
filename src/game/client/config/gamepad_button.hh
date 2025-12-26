// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: gamepad_button.hh
// Description: Gamepad button config value

#ifndef CLIENT_CONFIG_GAMEPAD_BUTTON_HH
#define CLIENT_CONFIG_GAMEPAD_BUTTON_HH
#pragma once

#include "core/config/ivalue.hh"

struct GLFWgamepadstate;

namespace config
{
class GamepadButton final : public IValue {
public:
    explicit GamepadButton(void);
    explicit GamepadButton(int button);
    virtual ~GamepadButton(void) = default;

    virtual std::string_view get(void) const override;
    virtual void set(std::string_view value) override;

    int get_button(void) const;
    void set_button(int button);

    bool equals(int button) const;
    bool is_pressed(const GLFWgamepadstate& state) const;

private:
    int m_gamepad_button;
    std::string_view m_name;
};
} // namespace config

#endif
