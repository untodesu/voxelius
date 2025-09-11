#pragma once

namespace math
{
template<typename T>
concept Arithmetic = std::is_arithmetic_v<T>;
template<typename T>
concept Integer = std::is_integral_v<T>;
template<typename T>
concept FloatingPoint = std::is_floating_point_v<T>;
} // namespace math
