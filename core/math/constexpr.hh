#pragma once

#include "core/math/concepts.hh"

namespace math
{
template<typename type, std::size_t size>
constexpr std::size_t array_size(const type (&)[size]);
} // namespace math

namespace math
{
template<std::integral scalar>
constexpr scalar log2(const scalar x);
template<std::signed_integral scalar>
constexpr scalar mod_signed(const scalar x, const scalar m);
template<math::signed_arithmetic result_scalar, math::arithmetic scalar>
constexpr result_scalar sign(const scalar x);
} // namespace math

namespace math
{
template<math::arithmetic scalar>
constexpr scalar degrees(const scalar x);
template<math::arithmetic scalar>
constexpr scalar radians(const scalar x);
} // namespace math

template<typename type, std::size_t size>
constexpr std::size_t math::array_size(const type (&)[size])
{
    return size;
}

template<std::integral scalar>
constexpr scalar math::log2(const scalar x)
{
    if(x < static_cast<scalar>(2))
        return static_cast<scalar>(0);
    return math::log2<scalar>((x + static_cast<scalar>(1)) >> 1) + static_cast<scalar>(1);
}

template<std::signed_integral scalar>
constexpr scalar math::mod_signed(const scalar x, const scalar m)
{
    auto result = static_cast<scalar>(x % m);
    if(result < static_cast<scalar>(0))
        return result + m;
    return result;
}

template<math::signed_arithmetic result_scalar, math::arithmetic scalar>
constexpr result_scalar math::sign(const scalar x)
{
    if(x < static_cast<scalar>(0))
        return static_cast<result_scalar>(-1);
    if(x > static_cast<scalar>(0))
        return static_cast<result_scalar>(+1);
    return static_cast<result_scalar>(0);
}

template<math::arithmetic scalar>
constexpr scalar math::degrees(const scalar x)
{
    return static_cast<scalar>(static_cast<double>(x) * 180.0 / M_PI);
}

template<math::arithmetic scalar>
constexpr scalar math::radians(const scalar x)
{
    return static_cast<scalar>(static_cast<double>(x) * M_PI / 180.0);
}
