#pragma once

#include "core/config/ivalue.hh"

struct GLFWgamepadstate;

namespace config
{
class GamepadAxis final : public IValue {
public:
    explicit GamepadAxis(void);
    explicit GamepadAxis(int axis, bool inverted);
    virtual ~GamepadAxis(void) = default;

    virtual std::string_view get(void) const override;
    virtual void set(std::string_view value) override;

    int get_axis(void) const;
    void set_axis(int axis);

    bool is_inverted(void) const;
    void set_inverted(bool inverted);

    float get_value(const GLFWgamepadstate& state, float deadzone = 0.0f) const;

    // Conventional get/set methods implemented by
    // this configuration value actually contain the
    // inversion flag. Since we're updating that flag
    // in the UI by means of a separate checkbox, we only need the name here
    std::string_view get_name(void) const;

private:
    bool m_inverted;
    int m_gamepad_axis;
    std::string m_full_string;
    std::string_view m_name;
};
} // namespace config
