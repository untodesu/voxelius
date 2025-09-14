#pragma once

namespace math
{
template<typename type>
concept arithmetic = std::is_arithmetic_v<type>;
template<typename type>
concept signed_arithmetic = std::is_arithmetic_v<type> && std::is_signed_v<type>;
template<typename type>
concept unsigned_arithmetic = std::is_arithmetic_v<type> && std::is_unsigned_v<type>;
} // namespace math
