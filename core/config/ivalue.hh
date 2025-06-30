#ifndef CORE_CONFIG_IVALUE_HH
#define CORE_CONFIG_IVALUE_HH 1
#pragma once

namespace config
{
class IValue {
public:
    virtual ~IValue(void) = default;
    virtual void set(const char* value) = 0;
    virtual const char* get(void) const = 0;
};
} // namespace config

#endif // CORE_CONFIG_IVALUE_HH
