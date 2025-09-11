#pragma once

#include "core/config/ivalue.hh"

namespace config
{
class KeyBind final : public IValue {
public:
    explicit KeyBind(void);
    explicit KeyBind(int default_value);
    virtual ~KeyBind(void) = default;

    virtual void set(std::string_view value) override;
    virtual std::string_view get(void) const override;

    void set_key(int keycode);
    int get_key(void) const;

    bool equals(int keycode) const;

private:
    std::string_view m_name;
    int m_glfw_keycode;
};
} // namespace config
