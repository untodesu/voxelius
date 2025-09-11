#include "core/pch.hh"

#include "core/config/boolean.hh"

config::Boolean::Boolean(bool default_value)
{
    m_value = default_value;
}

void config::Boolean::set(std::string_view value)
{
    m_value = from_string(value);
}

std::string_view config::Boolean::get(void) const
{
    return to_string(m_value);
}

bool config::Boolean::get_value(void) const
{
    return m_value;
}

void config::Boolean::set_value(bool value)
{
    m_value = value;
}

std::string_view config::Boolean::to_string(bool value)
{
    if(value) {
        return "true";
    }
    else {
        return "false";
    }
}

bool config::Boolean::from_string(std::string_view value)
{
    return value == "true" && value != "false";
}
