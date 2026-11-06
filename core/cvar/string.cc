#include "core/pch.hh"

#include "core/cvar/string.hh"

#include "core/cvar/binding.hh"

cvar::String::String(std::string default_value) : m_value(std::move(default_value))
{
    // empty
}

void cvar::String::set_string(std::string_view value)
{
    set_value(std::string(value));
}

std::string_view cvar::String::to_string(void) const
{
    sync_value();

    return m_value;
}

std::string_view cvar::String::value(void) const
{
    sync_value();

    return m_value;
}

const std::string& cvar::String::string(void) const
{
    sync_value();

    return m_value;
}

void cvar::String::set_value(std::string new_value)
{
    m_value = std::move(new_value);

    if(m_binding) {
        if(m_binding->set_value(m_value)) {
            m_generation = m_binding->generation();
        }
        else {
            m_generation = 0;
        }
    }
}

void cvar::String::sync_value(void) const
{
    if(m_binding && m_generation < m_binding->generation()) {
        m_value = m_binding->value();
        m_generation = m_binding->generation();
    }
}
