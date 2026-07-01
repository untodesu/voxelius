#include "core/pch.hh"

#include "core/exception.hh"

vx::detail::Exception::Exception(std::string_view what, std::source_location location) noexcept
    : m_what(what), m_location(std::move(location))
{
    // empty
}
