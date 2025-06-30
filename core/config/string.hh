#ifndef CORE_CONFIG_STRING_HH
#define CORE_CONFIG_STRING_HH 1
#pragma once

#include "core/config/ivalue.hh"

namespace config
{
class String : public IValue {
public:
    explicit String(const char* default_value);
    virtual ~String(void) = default;

    virtual void set(const char* value) override;
    virtual const char* get(void) const override;

private:
    std::string m_value;
};
} // namespace config

#endif // CORE_CONFIG_STRING_HH
