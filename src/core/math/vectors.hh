// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: vectors.hh
// Description: because NO ONE would POSSIBLY need integer-based distance calculations in a VOXEL game

#ifndef CORE_MATH_VECTORS_HH
#define CORE_MATH_VECTORS_HH
#pragma once

#include "core/math/concepts.hh"

namespace math
{
template<math::arithmetic T>
constexpr static inline const T length2(const glm::vec<2, T>& vector);
template<math::arithmetic T>
constexpr static inline const T length2(const glm::vec<3, T>& vector);
template<math::arithmetic T>
constexpr static inline const T distance2(const glm::vec<2, T>& vector_a, const glm::vec<2, T>& vector_b);
template<math::arithmetic T>
constexpr static inline const T distance2(const glm::vec<3, T>& vector_a, const glm::vec<3, T>& vector_b);
} // namespace math

template<math::arithmetic T>
constexpr static inline const T math::length2(const glm::vec<2, T>& vector)
{
    return (vector.x * vector.x) + (vector.y * vector.y);
}

template<math::arithmetic T>
constexpr static inline const T math::length2(const glm::vec<3, T>& vector)
{
    return (vector.x * vector.x) + (vector.y * vector.y) + (vector.z * vector.z);
}

template<math::arithmetic T>
constexpr static inline const T math::distance2(const glm::vec<2, T>& vector_a, const glm::vec<2, T>& vector_b)
{
    return math::length2(vector_a - vector_b);
}

template<math::arithmetic T>
constexpr static inline const T math::distance2(const glm::vec<3, T>& vector_a, const glm::vec<3, T>& vector_b)
{
    return math::length2(vector_a - vector_b);
}

#endif
