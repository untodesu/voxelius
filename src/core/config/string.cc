#include "core/pch.hh"

#include "core/config/string.hh"

config::String::String(std::string_view default_value)
{
    m_value = default_value;
}

void config::String::set(std::string_view value)
{
    m_value = value;
}

std::string_view config::String::get(void) const
{
    return m_value;
}
