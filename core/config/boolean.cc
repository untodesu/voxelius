#include "core/pch.hh"

#include "core/config/boolean.hh"

#include "core/config/arithmetic.hh"

bool config::Boolean::string_to_bool(std::string_view value)
{
    auto is_true = false;
    is_true = is_true || 0 == value.compare("true");
    is_true = is_true || 0 == value.compare("yes");
    is_true = is_true || 0 == value.compare("on");
    is_true = is_true || 0 == value.compare("1");

    auto is_false = false;
    is_false = is_false || 0 == value.compare("false");
    is_false = is_false || 0 == value.compare("off");
    is_false = is_false || 0 == value.compare("no");
    is_false = is_false || 0 == value.compare("0");

    return is_true && !is_false;
}

std::string_view config::Boolean::bool_to_string(bool value)
{
    if(value)
        return std::string_view("true");
    return std::string_view("false");
}

config::Boolean::Boolean(bool default_value) : m_boolean(default_value)
{
    // empty
}

void config::Boolean::set_boolean(bool value) noexcept
{
    m_boolean = value;
}

std::string_view config::Boolean::value(void) const noexcept
{
    return bool_to_string(m_boolean);
}

bool config::Boolean::set_value(std::string_view value) noexcept
{
    m_boolean = string_to_bool(value);

    return true;
}
