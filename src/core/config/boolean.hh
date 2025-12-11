#pragma once

#include "core/config/ivalue.hh"

namespace config
{
class Boolean : public IValue {
public:
    explicit Boolean(bool default_value = false);
    virtual ~Boolean(void) = default;

    virtual void set(std::string_view value) override;
    virtual std::string_view get(void) const override;

    bool get_value(void) const;
    void set_value(bool value);

private:
    bool m_value;

public:
    static std::string_view to_string(bool value);
    static bool from_string(std::string_view value);
};
} // namespace config
