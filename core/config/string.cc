#include "core/pch.hh"

#include "core/config/string.hh"

config::String::String(std::string_view default_value) : m_string(default_value)
{
    // empty
}

void config::String::set_string(std::string_view value)
{
    m_string = value;
}

std::string_view config::String::value(void) const noexcept
{
    return m_string;
}

bool config::String::set_value(std::string_view value) noexcept
{
    m_string = value;

    return true;
}
