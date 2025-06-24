#ifndef CORE_CONSTEXPR_HH
#define CORE_CONSTEXPR_HH 1
#pragma once

namespace cxpr
{
template<typename T>
constexpr static inline const T abs(const T x);
template<typename T, std::size_t L>
constexpr static inline const std::size_t array_size(const T (&)[L]);
template<typename T, typename F>
constexpr static inline const T ceil(const F x);
template<typename T>
constexpr static inline const T degrees(const T x);
template<typename T, typename F>
constexpr static inline const T floor(const F x);
template<typename T>
constexpr static inline const T clamp(const T x, const T min, const T max);
template<typename T, typename F>
constexpr static inline const T lerp(const T x, const T y, const F a);
template<typename T>
constexpr static inline const T log2(const T x);
template<typename T>
constexpr static inline const T max(const T x, const T y);
template<typename T>
constexpr static inline const T min(const T x, const T y);
template<typename T>
constexpr static inline const T mod_signed(const T x, const T m);
template<typename T>
constexpr static inline const T pow2(const T x);
template<typename T>
constexpr static inline const T radians(const T x);
template<typename T>
constexpr static inline const bool range(const T x, const T min, const T max);
template<typename T, typename F>
constexpr static inline const T sign(const F x);
template<typename T, typename F>
constexpr static inline const T smoothstep(const T x, const T y, const F a);
} // namespace cxpr

template<typename T>
constexpr static inline const T cxpr::abs(const T x)
{
    if(x < static_cast<T>(0)) {
        return -x;
    } else {
        return x;
    }
}

template<typename T, std::size_t L>
constexpr static inline const std::size_t cxpr::array_size(const T (&)[L])
{
    return L;
}

template<typename T, typename F>
constexpr static inline const T cxpr::ceil(const F x)
{
    static_assert(std::is_integral_v<T>);
    static_assert(std::is_floating_point_v<F>);

    const T ival = static_cast<T>(x);

    if(ival < x) {
        return ival + static_cast<T>(1);
    } else {
        return ival;
    }
}

template<typename T>
constexpr static inline const T cxpr::degrees(const T x)
{
    return x * static_cast<T>(180.0) / static_cast<T>(M_PI);
}

template<typename T, typename F>
constexpr static inline const T cxpr::floor(const F x)
{
    static_assert(std::is_integral_v<T>);
    static_assert(std::is_floating_point_v<F>);

    const T ival = static_cast<T>(x);

    if(ival > x) {
        return ival - static_cast<T>(1);
    } else {
        return ival;
    }
}

template<typename T>
constexpr static inline const T cxpr::clamp(const T x, const T min, const T max)
{
    if(x < min) {
        return min;
    } else if(x > max) {
        return max;
    } else {
        return x;
    }
}

template<typename T, typename F>
constexpr static inline const T cxpr::lerp(const T x, const T y, const F a)
{
    static_assert(std::is_arithmetic_v<T>);
    static_assert(std::is_floating_point_v<F>);
    return static_cast<T>(static_cast<F>(x) * (static_cast<F>(1.0f) - a) + static_cast<F>(y) * a);
}

template<typename T>
constexpr static inline const T cxpr::log2(const T x)
{
    if(x < 2) {
        return 0;
    } else {
        return cxpr::log2<T>((x + 1) >> 1) + 1;
    }
}

template<typename T>
constexpr static inline const T cxpr::max(const T x, const T y)
{
    if(x < y) {
        return y;
    } else {
        return x;
    }
}

template<typename T>
constexpr static inline const T cxpr::min(const T x, const T y)
{
    if(x > y) {
        return y;
    } else {
        return x;
    }
}

template<typename T>
constexpr static inline const T cxpr::mod_signed(const T x, const T m)
{
    static_assert(std::is_signed_v<T>);
    static_assert(std::is_integral_v<T>);
    auto result = static_cast<T>(x % m);

    if(result < T(0)) {
        return result + m;
    } else {
        return result;
    }
}

template<typename T>
constexpr static inline const T cxpr::pow2(const T x)
{
    T value = static_cast<T>(1);
    while(value < x)
        value *= static_cast<T>(2);
    return value;
}

template<typename T>
constexpr static inline const T cxpr::radians(const T x)
{
    return x * static_cast<T>(M_PI) / static_cast<T>(180.0);
}

template<typename T>
constexpr static inline const bool cxpr::range(const T x, const T min, const T max)
{
    return ((x >= min) && (x <= max));
}

template<typename T, typename F>
constexpr static inline const T cxpr::sign(const F x)
{
    if(x < F(0)) {
        return T(-1);
    } else if(x > F(0)) {
        return T(+1);
    } else {
        return T(0);
    }
}

template<typename T, typename F>
constexpr static inline const T cxpr::smoothstep(const T x, const T y, const F a)
{
    static_assert(std::is_arithmetic_v<T>);
    static_assert(std::is_floating_point_v<F>);

    const F t = cxpr::clamp<F>((a - x) / (y - x), F(0), F(1));
    return static_cast<T>(t * t * (F(3) - F(2) * t));
}

#endif /* CORE_CONSTEXPR_HH */
