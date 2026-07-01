#ifndef DCF25E76_20FC_4786_A27D_64AB9DFBD318
#define DCF25E76_20FC_4786_A27D_64AB9DFBD318

#include "core/frustum.hh"

class Camera final {
public:
    constexpr const Eigen::Matrix4f& view_projection(void) const noexcept;
    constexpr const Eigen::Matrix4f& projection(void) const noexcept;
    constexpr const Eigen::Matrix4f& view(void) const noexcept;
    constexpr const Frustum& frustum(void) const noexcept;

    Eigen::Vector3f position(void) const noexcept;
    Eigen::Vector3f forward_vector(void) const noexcept;
    Eigen::Vector3f right_vector(void) const noexcept;
    Eigen::Vector3f up_vector(void) const noexcept;

    Eigen::Quaternionf orientation(void) const noexcept;
    Eigen::Vector3f angles(void) const noexcept;

    void set_orthographic(float left, float right, float bottom, float top, float z_near, float z_far) noexcept;
    void set_perspective(float fov_y, float aspect_ratio, float z_near, float z_far) noexcept;

    void set_view(const Eigen::Vector3f& position, const Eigen::Quaternionf& orientation) noexcept;
    void set_view(const Eigen::Vector3f& position, const Eigen::Vector3f& angles) noexcept;

    void set_look(const Eigen::Vector3f& position, const Eigen::Vector3f& target) noexcept;
    void set_look(const Eigen::Vector3f& position, const Eigen::Vector3f& target, const Eigen::Vector3f& up) noexcept;

    void update(void) noexcept;

private:
    Eigen::Matrix4f m_view_projection { Eigen::Matrix4f::Constant(std::numeric_limits<float>::quiet_NaN()) };
    Eigen::Matrix4f m_projection { Eigen::Matrix4f::Constant(std::numeric_limits<float>::quiet_NaN()) };
    Eigen::Matrix4f m_view { Eigen::Matrix4f::Constant(std::numeric_limits<float>::quiet_NaN()) };
    Frustum m_frustum {};
};

constexpr const Eigen::Matrix4f& Camera::view_projection(void) const noexcept
{
    return m_view_projection;
}

constexpr const Eigen::Matrix4f& Camera::projection(void) const noexcept
{
    return m_projection;
}

constexpr const Eigen::Matrix4f& Camera::view(void) const noexcept
{
    return m_view;
}

constexpr const Frustum& Camera::frustum(void) const noexcept
{
    return m_frustum;
}

#endif /* DCF25E76_20FC_4786_A27D_64AB9DFBD318 */
