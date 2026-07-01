#ifndef AD54D621_66CB_4B08_A298_C58D9FD8CE86
#define AD54D621_66CB_4B08_A298_C58D9FD8CE86

namespace utils
{
float wrap_angle_360(float angle_rad) noexcept;
float wrap_angle_180(float angle_rad) noexcept;
} // namespace utils

namespace utils
{
constexpr float radians(float degrees) noexcept;
constexpr float degrees(float radians) noexcept;
} // namespace utils

namespace utils
{
template<typename T, std::size_t N>
constexpr Eigen::Vector<T, N> radians(const Eigen::Vector<T, N>& degrees) noexcept;
template<typename T, std::size_t N>
constexpr Eigen::Vector<T, N> degrees(const Eigen::Vector<T, N>& radians) noexcept;
} // namespace utils

constexpr float utils::radians(float degrees) noexcept
{
    return degrees * std::numbers::pi_v<float> / 180.0f;
}

constexpr float utils::degrees(float radians) noexcept
{
    return radians * 180.0f / std::numbers::pi_v<float>;
}

template<typename T, std::size_t N>
constexpr Eigen::Vector<T, N> utils::radians(const Eigen::Vector<T, N>& degrees) noexcept
{
    return degrees * static_cast<T>(std::numbers::pi_v<float> / 180.0f);
}

template<typename T, std::size_t N>
constexpr Eigen::Vector<T, N> utils::degrees(const Eigen::Vector<T, N>& radians) noexcept
{
    return radians * static_cast<T>(180.0f / std::numbers::pi_v<float>);
}

#endif /* AD54D621_66CB_4B08_A298_C58D9FD8CE86 */
