#ifndef CLIENT_GAMEPAD_BUTTON_HH
#define CLIENT_GAMEPAD_BUTTON_HH 1
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

    virtual const char* get(void) const override;
    virtual void set(const char* value) override;

    int get_button(void) const;
    void set_button(int button);

    bool equals(int button) const;
    bool is_pressed(const GLFWgamepadstate& state) const;

private:
    int m_gamepad_button;
    const char* m_name;
};
} // namespace config

#endif // CLIENT_GAMEPAD_BUTTON_HH
