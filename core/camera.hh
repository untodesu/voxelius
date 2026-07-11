#ifndef DCF25E76_20FC_4786_A27D_64AB9DFBD318
#define DCF25E76_20FC_4786_A27D_64AB9DFBD318

#include "core/frustum.hh"

class Camera final {
public:
    constexpr const Eigen::Matrix4f& view_projection(void) const;
    constexpr const Eigen::Matrix4f& projection(void) const;
    constexpr const Eigen::Matrix4f& view(void) const;
    constexpr const Frustum& frustum(void) const;

    Eigen::Vector3f position(void) const;
    Eigen::Vector3f forward_vector(void) const;
    Eigen::Vector3f right_vector(void) const;
    Eigen::Vector3f up_vector(void) const;

    Eigen::Quaternionf orientation(void) const;
    Eigen::Vector3f angles(void) const;

    void set_orthographic(float left, float right, float bottom, float top, float z_near, float z_far);
    void set_perspective(float fov_y, float aspect_ratio, float z_near, float z_far);

    void set_view(const Eigen::Vector3f& position, const Eigen::Quaternionf& orientation);
    void set_view(const Eigen::Vector3f& position, const Eigen::Vector3f& angles);

    void set_look(const Eigen::Vector3f& position, const Eigen::Vector3f& target);
    void set_look(const Eigen::Vector3f& position, const Eigen::Vector3f& target, const Eigen::Vector3f& up);

    void update(void);

private:
    Eigen::Matrix4f m_view_projection { Eigen::Matrix4f::Constant(std::numeric_limits<float>::quiet_NaN()) };
    Eigen::Matrix4f m_projection { Eigen::Matrix4f::Constant(std::numeric_limits<float>::quiet_NaN()) };
    Eigen::Matrix4f m_view { Eigen::Matrix4f::Constant(std::numeric_limits<float>::quiet_NaN()) };
    Frustum m_frustum {};
};

constexpr const Eigen::Matrix4f& Camera::view_projection(void) const
{
    return m_view_projection;
}

constexpr const Eigen::Matrix4f& Camera::projection(void) const
{
    return m_projection;
}

constexpr const Eigen::Matrix4f& Camera::view(void) const
{
    return m_view;
}

constexpr const Frustum& Camera::frustum(void) const
{
    return m_frustum;
}

#endif /* DCF25E76_20FC_4786_A27D_64AB9DFBD318 */
