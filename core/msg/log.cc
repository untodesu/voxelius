#include "core/pch.hh"

#include "core/msg/log.hh"

void msg::detail::log(level_type level, std::source_location location, std::string_view message)
{
    Record record {};
    record.level = level;
    record.where = std::move(location);
    record.message = message;

    submit(record);
}

void msg::log_debug(std::string_view message, std::source_location location)
{
#ifndef NDEBUG
    detail::log(LEVEL_DEBUG, std::move(location), message);
#endif
}

void msg::log_info(std::string_view message, std::source_location location)
{
    detail::log(LEVEL_INFO, std::move(location), message);
}

void msg::log_warning(std::string_view message, std::source_location location)
{
    detail::log(LEVEL_WARNING, std::move(location), message);
}

void msg::log_error(std::string_view message, std::source_location location)
{
    detail::log(LEVEL_ERROR, std::move(location), message);
}

void msg::log_critical(std::string_view message, std::source_location location)
{
    detail::log(LEVEL_CRITICAL, std::move(location), message);
}
