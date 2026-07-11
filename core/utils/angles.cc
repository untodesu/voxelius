#include "core/pch.hh"

#include "core/utils/angles.hh"

constexpr static auto TAU = 2.0f * std::numbers::pi_v<float>;

float utils::wrap_angle_360(float angle_rad)
{
    return std::fmod(std::fmod(angle_rad, TAU) + TAU, TAU);
}

float utils::wrap_angle_180(float angle_rad)
{
    angle_rad = std::fmod(angle_rad + std::numbers::pi_v<float>, TAU);

    if(angle_rad < 0.0f) {
        angle_rad += TAU;
    }

    return angle_rad - std::numbers::pi_v<float>;
}

Eigen::Vector3f utils::wrap_angle_360(Eigen::Vector3f angle_rad)
{
    angle_rad[0] = wrap_angle_360(angle_rad[0]);
    angle_rad[1] = wrap_angle_360(angle_rad[1]);
    angle_rad[2] = wrap_angle_360(angle_rad[2]);
    return std::move(angle_rad);
}

Eigen::Vector3f utils::wrap_angle_180(Eigen::Vector3f angle_rad)
{
    angle_rad[0] = wrap_angle_180(angle_rad[0]);
    angle_rad[1] = wrap_angle_180(angle_rad[1]);
    angle_rad[2] = wrap_angle_180(angle_rad[2]);
    return std::move(angle_rad);
}
