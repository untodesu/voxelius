#ifndef CORE_VECTORS_HH
#define CORE_VECTORS_HH 1
#pragma once

#include "core/concepts.hh"

// core/vectors.hh - because NO ONE would POSSIBLY
// need integer-based distance calculations in a
// game about voxels. That would be INSANE! :D

namespace vx
{
template<vx::Arithmetic T>
constexpr static inline const T length2(const glm::vec<2, T>& vector);
template<vx::Arithmetic T>
constexpr static inline const T length2(const glm::vec<3, T>& vector);
template<vx::Arithmetic T>
constexpr static inline const T distance2(const glm::vec<2, T>& vector_a, const glm::vec<2, T>& vector_b);
template<vx::Arithmetic T>
constexpr static inline const T distance2(const glm::vec<3, T>& vector_a, const glm::vec<3, T>& vector_b);
} // namespace vx

template<vx::Arithmetic T>
constexpr static inline const T vx::length2(const glm::vec<2, T>& vector)
{
    return (vector.x * vector.x) + (vector.y * vector.y);
}

template<vx::Arithmetic T>
constexpr static inline const T vx::length2(const glm::vec<3, T>& vector)
{
    return (vector.x * vector.x) + (vector.y * vector.y) + (vector.z * vector.z);
}

template<vx::Arithmetic T>
constexpr static inline const T vx::distance2(const glm::vec<2, T>& vector_a, const glm::vec<2, T>& vector_b)
{
    return vx::length2(vector_a - vector_b);
}

template<vx::Arithmetic T>
constexpr static inline const T vx::distance2(const glm::vec<3, T>& vector_a, const glm::vec<3, T>& vector_b)
{
    return vx::length2(vector_a - vector_b);
}

#endif // CORE_VECTORS_HH
