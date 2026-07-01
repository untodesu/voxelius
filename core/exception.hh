#ifndef B843EAA9_60C3_4C2B_8036_DF1026035AA8
#define B843EAA9_60C3_4C2B_8036_DF1026035AA8

namespace vx::detail
{
template<typename... Args>
struct FormatWithLocation final {
    std::format_string<Args...> value;
    std::source_location location;

    template<typename StringT>
    consteval FormatWithLocation(const StringT& value, std::source_location location = std::source_location::current())
        : value(value), location(std::move(location))
    {
        // empty
    }
};
} // namespace vx::detail

namespace vx::detail
{
class Exception {
public:
    template<typename... Args>
    explicit Exception(FormatWithLocation<typename std::type_identity<Args>::type...> fmt, Args&&... args) noexcept;
    explicit Exception(std::string_view what, std::source_location location = std::source_location::current()) noexcept;
    virtual ~Exception(void) noexcept = default;

    constexpr std::string_view what(void) const noexcept;
    constexpr const char* what_standard(void) const noexcept;
    constexpr const std::source_location& location(void) const noexcept;

private:
    std::string m_what;
    std::source_location m_location;
};
} // namespace vx::detail

namespace vx::detail
{
template<typename Tag>
class TaggedException final : public Exception {
public:
    using Exception::Exception;
};
} // namespace vx::detail

namespace vx::detail
{
template<typename T>
concept derived_exception = std::derived_from<T, Exception>;
} // namespace vx::detail

namespace vx
{
using runtime_error = detail::TaggedException<struct runtime_error_tag>;
using argument_error = detail::TaggedException<struct argument_error_tag>;
using range_error = detail::TaggedException<struct range_error_tag>;
} // namespace vx

namespace vx
{
template<detail::derived_exception T = runtime_error>
void throw_if(bool condition, std::source_location location = std::source_location::current());
template<detail::derived_exception T = runtime_error>
void throw_if_not(bool condition, std::source_location location = std::source_location::current());
} // namespace vx

namespace vx
{
template<detail::derived_exception T = runtime_error>
void throw_if(bool condition, std::string_view what, std::source_location location = std::source_location::current());
template<detail::derived_exception T = runtime_error>
void throw_if_not(bool condition, std::string_view what, std::source_location location = std::source_location::current());
} // namespace vx

namespace vx
{
template<detail::derived_exception T = runtime_error, typename... Args>
void throw_if_fmt(bool condition, detail::FormatWithLocation<typename std::type_identity<Args>::type...> fmt, Args&&... args);
template<detail::derived_exception T = runtime_error, typename... Args>
void throw_if_not_fmt(bool condition, detail::FormatWithLocation<typename std::type_identity<Args>::type...> fmt, Args&&... args);
} // namespace vx

template<typename... Args>
vx::detail::Exception::Exception(detail::FormatWithLocation<typename std::type_identity<Args>::type...> fmt, Args&&... args) noexcept
    : m_what(std::vformat(fmt.value.get(), std::make_format_args(args...))), m_location(std::move(fmt.location))
{
    // empty
}

constexpr std::string_view vx::detail::Exception::what(void) const noexcept
{
    return m_what;
}

constexpr const char* vx::detail::Exception::what_standard(void) const noexcept
{
    return m_what.c_str();
}

constexpr const std::source_location& vx::detail::Exception::location(void) const noexcept
{
    return m_location;
}

template<vx::detail::derived_exception T>
void vx::throw_if(bool condition, std::source_location location)
{
    if(condition) {
        throw T("unspecified fail condition", std::move(location));
    }
}

template<vx::detail::derived_exception T>
void vx::throw_if_not(bool condition, std::source_location location)
{
    if(!condition) {
        throw T("unspecified fail condition", std::move(location));
    }
}

template<vx::detail::derived_exception T>
void vx::throw_if(bool condition, std::string_view what, std::source_location location)
{
    if(condition) {
        throw T(what, std::move(location));
    }
}

template<vx::detail::derived_exception T>
void vx::throw_if_not(bool condition, std::string_view what, std::source_location location)
{
    if(!condition) {
        throw T(what, std::move(location));
    }
}

template<vx::detail::derived_exception T, typename... Args>
void vx::throw_if_fmt(bool condition, detail::FormatWithLocation<typename std::type_identity<Args>::type...> fmt, Args&&... args)
{
    if(condition) {
        throw T(std::move(fmt), std::forward<Args>(args)...);
    }
}

template<vx::detail::derived_exception T, typename... Args>
void vx::throw_if_not_fmt(bool condition, detail::FormatWithLocation<typename std::type_identity<Args>::type...> fmt, Args&&... args)
{
    if(!condition) {
        throw T(std::move(fmt), std::forward<Args>(args)...);
    }
}

#endif /* B843EAA9_60C3_4C2B_8036_DF1026035AA8 */
