#include "core/pch.hh"

#include "core/config/string.hh"

config::String::String(const char* default_value)
{
    m_value = default_value;
}

void config::String::set(const char* value)
{
    m_value = value;
}

const char* config::String::get(void) const
{
    return m_value.c_str();
}
