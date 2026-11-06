#ifndef BB08B180_E85C_4711_9925_4A70EC106E2F
#define BB08B180_E85C_4711_9925_4A70EC106E2F

namespace vx
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
} // namespace vx

#endif /* BB08B180_E85C_4711_9925_4A70EC106E2F */
