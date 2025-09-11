#ifndef CORE_CONFIG_STRING_HH
#define CORE_CONFIG_STRING_HH 1
#pragma once

#include "core/config/ivalue.hh"

namespace config
{
class String : public IValue {
public:
    explicit String(std::string_view default_value);
    virtual ~String(void) = default;

    virtual void set(std::string_view value) override;
    virtual std::string_view get(void) const override;

    constexpr const std::string& get_value(void) const noexcept;
    constexpr const char* c_str(void) const noexcept;

private:
    std::string m_value;
};
} // namespace config

constexpr const std::string& config::String::get_value(void) const noexcept
{
    return m_value;
}

constexpr const char* config::String::c_str(void) const noexcept
{
    return m_value.c_str();
}

#endif // CORE_CONFIG_STRING_HH
