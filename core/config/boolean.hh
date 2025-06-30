#ifndef CORE_CONFIG_BOOLEAN_HH
#define CORE_CONFIG_BOOLEAN_HH 1
#pragma once

#include "core/config/ivalue.hh"

namespace config
{
class Boolean : public IValue {
public:
    explicit Boolean(bool default_value = false);
    virtual ~Boolean(void) = default;

    virtual void set(const char* value) override;
    virtual const char* get(void) const override;

    bool get_value(void) const;
    void set_value(bool value);

private:
    bool m_value;

public:
    static const char* to_string(bool value);
    static bool from_string(const char* value);
};
} // namespace config

#endif // CORE_CONFIG_BOOLEAN_HH
