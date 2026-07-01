#include "core/pch.hh"

#include "core/utils/angles.hh"

constexpr static auto TAU = 2.0f * std::numbers::pi_v<float>;

float utils::wrap_angle_360(float angle_rad) noexcept
{
    angle_rad = std::fmod(angle_rad, TAU);

    if(angle_rad < 0.0f) {
        angle_rad += TAU;
    }

    return angle_rad;
}

float utils::wrap_angle_180(float angle_rad) noexcept
{
    angle_rad = std::fmod(angle_rad + std::numbers::pi_v<float>, TAU);

    if(angle_rad < 0.0f) {
        angle_rad += TAU;
    }

    return angle_rad - std::numbers::pi_v<float>;
}
