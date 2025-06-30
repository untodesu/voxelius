#ifndef CLIENT_KEYBIND_HH
#define CLIENT_KEYBIND_HH 1
#pragma once

#include "core/config/ivalue.hh"

namespace config
{
class KeyBind final : public IValue {
public:
    explicit KeyBind(void);
    explicit KeyBind(int default_value);
    virtual ~KeyBind(void) = default;

    virtual void set(const char* value) override;
    virtual const char* get(void) const override;

    void set_key(int keycode);
    int get_key(void) const;

    bool equals(int keycode) const;

private:
    const char* m_name;
    int m_glfw_keycode;
};
} // namespace config

#endif // CLIENT_KEYBIND_HH
