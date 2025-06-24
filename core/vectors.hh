#ifndef CORE_VECTORS_HH
#define CORE_VECTORS_HH 1
#pragma once

// cxvectors.hh - because NO ONE would POSSIBLY
// need integer-based distance calculations in a
// game about voxels. That would be INSANE! :D

namespace cxvectors
{
template<typename value_type>
constexpr static inline const value_type length2(const glm::vec<2, value_type>& vector);
template<typename value_type>
constexpr static inline const value_type length2(const glm::vec<3, value_type>& vector);
template<typename value_type>
constexpr static inline const value_type distance2(const glm::vec<2, value_type>& vector_a, const glm::vec<2, value_type>& vector_b);
template<typename value_type>
constexpr static inline const value_type distance2(const glm::vec<3, value_type>& vector_a, const glm::vec<3, value_type>& vector_b);
} // namespace cxvectors

template<typename value_type>
constexpr static inline const value_type cxvectors::length2(const glm::vec<2, value_type>& vector)
{
    static_assert(std::is_arithmetic_v<value_type>);
    return (vector.x * vector.x) + (vector.y * vector.y);
}

template<typename value_type>
constexpr static inline const value_type cxvectors::length2(const glm::vec<3, value_type>& vector)
{
    static_assert(std::is_arithmetic_v<value_type>);
    return (vector.x * vector.x) + (vector.y * vector.y) + (vector.z * vector.z);
}

template<typename value_type>
constexpr static inline const value_type cxvectors::distance2(const glm::vec<2, value_type>& vector_a, const glm::vec<2, value_type>& vector_b)
{
    static_assert(std::is_arithmetic_v<value_type>);
    return cxvectors::length2(vector_a - vector_b);
}

template<typename value_type>
constexpr static inline const value_type cxvectors::distance2(const glm::vec<3, value_type>& vector_a, const glm::vec<3, value_type>& vector_b)
{
    static_assert(std::is_arithmetic_v<value_type>);
    return cxvectors::length2(vector_a - vector_b);
}

#endif /* CORE_VECTORS_HH */
