#ifndef CORE_MATH_CONSTEXPR_HH
#define CORE_MATH_CONSTEXPR_HH 1
#pragma once

#include "core/math/concepts.hh"

namespace math
{
template<math::Arithmetic T>
constexpr static inline const T abs(const T x);
template<typename T, std::size_t L>
constexpr static inline const std::size_t array_size(const T (&)[L]);
template<math::Integer T, math::FloatingPoint F>
constexpr static inline const T ceil(const F x);
template<math::Arithmetic T>
constexpr static inline const T degrees(const T x);
template<math::Integer T, math::FloatingPoint F>
constexpr static inline const T floor(const F x);
template<math::Arithmetic T>
constexpr static inline const T clamp(const T x, const T min, const T max);
template<math::Arithmetic T, math::FloatingPoint F>
constexpr static inline const T lerp(const T x, const T y, const F a);
template<math::Arithmetic T>
constexpr static inline const T log2(const T x);
template<math::Arithmetic T>
constexpr static inline const T max(const T x, const T y);
template<math::Arithmetic T>
constexpr static inline const T min(const T x, const T y);
template<math::Integer T>
requires std::is_signed_v<T>
constexpr static inline const T mod_signed(const T x, const T m);
template<math::Arithmetic T>
constexpr static inline const T pow2(const T x);
template<math::Arithmetic T>
constexpr static inline const T radians(const T x);
template<math::Arithmetic T>
constexpr static inline const bool range(const T x, const T min, const T max);
template<math::Arithmetic T, math::FloatingPoint F>
constexpr static inline const T sign(const F x);
template<math::Arithmetic T, math::FloatingPoint F>
constexpr static inline const T smoothstep(const T x, const T y, const F a);
} // namespace math

template<math::Arithmetic T>
constexpr static inline const T math::abs(const T x)
{
    if(x < static_cast<T>(0)) {
        return -x;
    } else {
        return x;
    }
}

template<typename T, std::size_t L>
constexpr static inline const std::size_t math::array_size(const T (&)[L])
{
    return L;
}

template<math::Integer T, math::FloatingPoint F>
constexpr static inline const T math::ceil(const F x)
{
    const T ival = static_cast<T>(x);

    if(ival < x) {
        return ival + static_cast<T>(1);
    } else {
        return ival;
    }
}

template<math::Arithmetic T>
constexpr static inline const T math::degrees(const T x)
{
    return x * static_cast<T>(180.0) / static_cast<T>(M_PI);
}

template<math::Integer T, math::FloatingPoint F>
constexpr static inline const T math::floor(const F x)
{
    const T ival = static_cast<T>(x);

    if(ival > x) {
        return ival - static_cast<T>(1);
    } else {
        return ival;
    }
}

template<math::Arithmetic T>
constexpr static inline const T math::clamp(const T x, const T min, const T max)
{
    if(x < min) {
        return min;
    } else if(x > max) {
        return max;
    } else {
        return x;
    }
}

template<math::Arithmetic T, math::FloatingPoint F>
constexpr static inline const T math::lerp(const T x, const T y, const F a)
{
    return static_cast<T>(static_cast<F>(x) * (static_cast<F>(1.0f) - a) + static_cast<F>(y) * a);
}

template<math::Arithmetic T>
constexpr static inline const T math::log2(const T x)
{
    if(x < 2) {
        return 0;
    } else {
        return math::log2<T>((x + 1) >> 1) + 1;
    }
}

template<math::Arithmetic T>
constexpr static inline const T math::max(const T x, const T y)
{
    if(x < y) {
        return y;
    } else {
        return x;
    }
}

template<math::Arithmetic T>
constexpr static inline const T math::min(const T x, const T y)
{
    if(x > y) {
        return y;
    } else {
        return x;
    }
}

template<math::Integer T>
requires std::is_signed_v<T>
constexpr static inline const T math::mod_signed(const T x, const T m)
{
    auto result = static_cast<T>(x % m);

    if(result < T(0)) {
        return result + m;
    } else {
        return result;
    }
}

template<math::Arithmetic T>
constexpr static inline const T math::pow2(const T x)
{
    T value = static_cast<T>(1);
    while(value < x)
        value *= static_cast<T>(2);
    return value;
}

template<math::Arithmetic T>
constexpr static inline const T math::radians(const T x)
{
    return x * static_cast<T>(M_PI) / static_cast<T>(180.0);
}

template<math::Arithmetic T>
constexpr static inline const bool math::range(const T x, const T min, const T max)
{
    return ((x >= min) && (x <= max));
}

template<math::Arithmetic T, math::FloatingPoint F>
constexpr static inline const T math::sign(const F x)
{
    if(x < F(0)) {
        return T(-1);
    } else if(x > F(0)) {
        return T(+1);
    } else {
        return T(0);
    }
}

template<math::Arithmetic T, math::FloatingPoint F>
constexpr static inline const T math::smoothstep(const T x, const T y, const F a)
{
    const F t = math::clamp<F>((a - x) / (y - x), F(0), F(1));
    return static_cast<T>(t * t * (F(3) - F(2) * t));
}

#endif // CORE_MATH_CONSTEXPR_HH
