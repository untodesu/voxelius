#pragma once

namespace config
{
class IValue {
public:
    virtual ~IValue(void) = default;
    virtual void set(std::string_view value) = 0;
    virtual std::string_view get(void) const = 0;
};
} // namespace config
