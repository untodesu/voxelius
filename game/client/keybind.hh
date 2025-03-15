#ifndef CLIENT_KEYBIND_HH
#define CLIENT_KEYBIND_HH 1
#pragma once

#include "core/config.hh"

class ConfigKeyBind final : public IConfigValue {
public:
    explicit ConfigKeyBind(void);
    explicit ConfigKeyBind(int default_value);
    virtual ~ConfigKeyBind(void) = default;

    virtual void set(const char *value) override;
    virtual const char *get(void) const override;

    void set_key(int keycode);
    int get_key(void) const;

    bool equals(int keycode) const;

private:
    const char *m_name;
    int m_glfw_keycode;
};

#endif /* CLIENT_KEYBIND_HH */
