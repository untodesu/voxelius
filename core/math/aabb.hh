#pragma once

#include "core/math/concepts.hh"

namespace math
{
template<Arithmetic T>
class AABB {
public:
    using value_type = T;
    using vector_type = glm::vec<3, T>;

    constexpr AABB(void) = default;
    constexpr explicit AABB(const vector_type& start, const vector_type& end);
    virtual ~AABB(void) = default;

    constexpr void set_bounds(const vector_type& start, const vector_type& end);
    constexpr void set_offset(const vector_type& base, const vector_type& size);

    constexpr const vector_type& get_min(void) const;
    constexpr const vector_type& get_max(void) const;

    constexpr bool contains(const vector_type& point) const;
    constexpr bool intersect(const AABB<value_type>& other_box) const;

    constexpr AABB<value_type> combine(const AABB<value_type>& other_box) const;
    constexpr AABB<value_type> multiply(const AABB<value_type>& other_box) const;
    constexpr AABB<value_type> push(const vector_type& vector) const;

public:
    vector_type min {};
    vector_type max {};
};
} // namespace math

namespace math
{
using AABBf = AABB<float>;
} // namespace math

template<math::Arithmetic T>
constexpr math::AABB<T>::AABB(const vector_type& start, const vector_type& end)
{
    set_bounds(start, end);
}

template<math::Arithmetic T>
constexpr void math::AABB<T>::set_bounds(const vector_type& start, const vector_type& end)
{
    min = start;
    max = end;
}

template<math::Arithmetic T>
constexpr void math::AABB<T>::set_offset(const vector_type& base, const vector_type& size)
{
    min = base;
    max = base + size;
}

template<math::Arithmetic T>
constexpr const typename math::AABB<T>::vector_type& math::AABB<T>::get_min(void) const
{
    return min;
}

template<math::Arithmetic T>
constexpr const typename math::AABB<T>::vector_type& math::AABB<T>::get_max(void) const
{
    return max;
}

template<math::Arithmetic T>
constexpr bool math::AABB<T>::contains(const vector_type& point) const
{
    auto result = true;
    result = result && (point.x >= min.x) && (point.x <= max.x);
    result = result && (point.y >= min.y) && (point.y <= max.y);
    result = result && (point.z >= min.z) && (point.z <= max.z);
    return result;
}

template<math::Arithmetic T>
constexpr bool math::AABB<T>::intersect(const AABB<value_type>& other_box) const
{
    auto result = true;
    result = result && (min.x <= other_box.max.x) && (max.x >= other_box.min.x);
    result = result && (min.y <= other_box.max.y) && (max.y >= other_box.min.y);
    result = result && (min.z <= other_box.max.z) && (max.z >= other_box.min.z);
    return result;
}

template<math::Arithmetic T>
constexpr math::AABB<T> math::AABB<T>::combine(const AABB<value_type>& other_box) const
{
    AABB<value_type> result;
    result.min.x = glm::min(min.x, other_box.min.x);
    result.min.y = glm::min(min.y, other_box.min.y);
    result.min.z = glm::min(min.z, other_box.min.z);
    result.max.x = glm::max(max.x, other_box.max.x);
    result.max.y = glm::max(max.y, other_box.max.y);
    result.max.z = glm::max(max.z, other_box.max.z);
    return result;
}

template<math::Arithmetic T>
constexpr math::AABB<T> math::AABB<T>::multiply(const AABB<value_type>& other_box) const
{
    AABB<value_type> result;
    result.min.x = glm::max(min.x, other_box.min.x);
    result.min.y = glm::max(min.y, other_box.min.y);
    result.min.z = glm::max(min.z, other_box.min.z);
    result.max.x = glm::min(max.x, other_box.max.x);
    result.max.y = glm::min(max.y, other_box.max.y);
    result.max.z = glm::min(max.z, other_box.max.z);
    return result;
}

template<math::Arithmetic T>
constexpr math::AABB<T> math::AABB<T>::push(const vector_type& vector) const
{
    AABB<value_type> result;
    result.min = min + vector;
    result.max = max + vector;
    return result;
}
