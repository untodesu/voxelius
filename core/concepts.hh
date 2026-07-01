#ifndef B3BD8C50_B2AB_408F_9D63_39DF88D33E82
#define B3BD8C50_B2AB_408F_9D63_39DF88D33E82

namespace vx
{
template<typename T>
concept arithmetic = std::is_arithmetic_v<T>;
template<typename T>
concept signed_arithmetic = std::is_arithmetic_v<T> && std::is_signed_v<T>;
template<typename T>
concept unsigned_arithmetic = std::is_arithmetic_v<T> && std::is_unsigned_v<T>;
} // namespace vx

namespace vx
{
template<typename T>
concept char_type = std::same_as<T, char> || std::same_as<T, wchar_t> || std::same_as<T, char8_t> || std::same_as<T, char16_t>
    || std::same_as<T, char32_t>;
} // namespace vx

namespace vx
{
template<typename T>
concept derived_exception = std::derived_from<T, std::runtime_error> || std::derived_from<T, std::logic_error>;
} // namespace vx

#endif /* B3BD8C50_B2AB_408F_9D63_39DF88D33E82 */
