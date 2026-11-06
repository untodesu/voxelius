#ifndef A1FD58A8_98A0_441B_9154_9A12BAFD3F1E
#define A1FD58A8_98A0_441B_9154_9A12BAFD3F1E

#include "core/format.hh"
#include "core/msg/sink.hh"

namespace msg::detail
{
template<typename... Args>
void log(level_type level, vx::FormatWithLocation<typename std::type_identity<Args>::type...> fmt, Args&&... args);
void log(level_type level, std::source_location location, std::string_view message);
} // namespace msg::detail

namespace msg
{
template<typename... Args>
void log_debug(vx::FormatWithLocation<typename std::type_identity<Args>::type...> fmt, Args&&... args);
template<typename... Args>
void log_info(vx::FormatWithLocation<typename std::type_identity<Args>::type...> fmt, Args&&... args);
template<typename... Args>
void log_warning(vx::FormatWithLocation<typename std::type_identity<Args>::type...> fmt, Args&&... args);
template<typename... Args>
void log_error(vx::FormatWithLocation<typename std::type_identity<Args>::type...> fmt, Args&&... args);
template<typename... Args>
void log_critical(vx::FormatWithLocation<typename std::type_identity<Args>::type...> fmt, Args&&... args);
} // namespace msg

namespace msg
{
void log_debug(std::string_view message, std::source_location location = std::source_location::current());
void log_info(std::string_view message, std::source_location location = std::source_location::current());
void log_warning(std::string_view message, std::source_location location = std::source_location::current());
void log_error(std::string_view message, std::source_location location = std::source_location::current());
void log_critical(std::string_view message, std::source_location location = std::source_location::current());
} // namespace msg

template<typename... Args>
void msg::detail::log(level_type level, vx::FormatWithLocation<typename std::type_identity<Args>::type...> fmt, Args&&... args)
{
    auto message = std::format(fmt.value, std::forward<Args>(args)...);

    log(level, std::move(fmt.location), std::string_view(message));
}

template<typename... Args>
void msg::log_debug(vx::FormatWithLocation<typename std::type_identity<Args>::type...> fmt, Args&&... args)
{
#ifndef NDEBUG
    detail::log(LEVEL_DEBUG, std::move(fmt), std::forward<Args>(args)...);
#endif
}

template<typename... Args>
void msg::log_info(vx::FormatWithLocation<typename std::type_identity<Args>::type...> fmt, Args&&... args)
{
    detail::log(LEVEL_INFO, std::move(fmt), std::forward<Args>(args)...);
}

template<typename... Args>
void msg::log_warning(vx::FormatWithLocation<typename std::type_identity<Args>::type...> fmt, Args&&... args)
{
    detail::log(LEVEL_WARNING, std::move(fmt), std::forward<Args>(args)...);
}

template<typename... Args>
void msg::log_error(vx::FormatWithLocation<typename std::type_identity<Args>::type...> fmt, Args&&... args)
{
    detail::log(LEVEL_ERROR, std::move(fmt), std::forward<Args>(args)...);
}

template<typename... Args>
void msg::log_critical(vx::FormatWithLocation<typename std::type_identity<Args>::type...> fmt, Args&&... args)
{
    detail::log(LEVEL_CRITICAL, std::move(fmt), std::forward<Args>(args)...);
}

#endif /* A1FD58A8_98A0_441B_9154_9A12BAFD3F1E */
