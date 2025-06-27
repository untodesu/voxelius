#ifndef CORE_ANGLES_HH
#define CORE_ANGLES_HH 1
#pragma once

#include "core/constexpr.hh"

constexpr float A180 = vx::radians(180.0f);
constexpr float A360 = vx::radians(360.0f);

namespace cxangles
{
float wrap_180(float angle);
float wrap_360(float angle);
} // namespace cxangles

namespace cxangles
{
glm::fvec3 wrap_180(const glm::fvec3& angles);
glm::fvec3 wrap_360(const glm::fvec3& angles);
} // namespace cxangles

namespace cxangles
{
void vectors(const glm::fvec3& angles, glm::fvec3& forward);
void vectors(const glm::fvec3& angles, glm::fvec3* forward, glm::fvec3* right, glm::fvec3* up);
} // namespace cxangles

inline float cxangles::wrap_180(float angle)
{
    const auto result = std::fmod(angle + A180, A360);

    if(result < 0.0f) {
        return result + A180;
    }

    return result - A180;
}

inline float cxangles::wrap_360(float angle)
{
    return std::fmod(std::fmod(angle, A360) + A360, A360);
}

inline glm::fvec3 cxangles::wrap_180(const glm::fvec3& angles)
{
    return glm::fvec3 {
        cxangles::wrap_180(angles.x),
        cxangles::wrap_180(angles.y),
        cxangles::wrap_180(angles.z),
    };
}

inline glm::fvec3 cxangles::wrap_360(const glm::fvec3& angles)
{
    return glm::fvec3 {
        cxangles::wrap_360(angles.x),
        cxangles::wrap_360(angles.y),
        cxangles::wrap_360(angles.z),
    };
}

inline void cxangles::vectors(const glm::fvec3& angles, glm::fvec3& forward)
{
    const float cosp = std::cos(angles.x);
    const float cosy = std::cos(angles.y);
    const float sinp = std::sin(angles.x);
    const float siny = std::sin(angles.y);

    forward.x = cosp * siny * (-1.0f);
    forward.y = sinp;
    forward.z = cosp * cosy * (-1.0f);
}

inline void cxangles::vectors(const glm::fvec3& angles, glm::fvec3* forward, glm::fvec3* right, glm::fvec3* up)
{
    if(!forward && !right && !up) {
        // There's no point in figuring out
        // direction vectors if nothing is passed
        // in the function to store that stuff in
        return;
    }

    const auto pcv = glm::cos(angles);
    const auto psv = glm::sin(angles);
    const auto ncv = pcv * (-1.0f);
    const auto nsv = psv * (-1.0f);

    if(forward) {
        forward->x = pcv.x * nsv.y;
        forward->y = psv.x;
        forward->z = pcv.x * ncv.y;
    }

    if(right) {
        right->x = pcv.z * pcv.y;
        right->y = psv.z * pcv.y;
        right->z = nsv.y;
    }

    if(up) {
        up->x = psv.x * psv.y * pcv.z + ncv.y * psv.z;
        up->y = pcv.x * pcv.z;
        up->z = nsv.x * ncv.y * pcv.z + psv.y * psv.z;
    }
}

#endif /* CORE_ANGLES_HH */
