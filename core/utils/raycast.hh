#ifndef DBAF77A6_5F41_42F5_BA50_B3D011623D5C
#define DBAF77A6_5F41_42F5_BA50_B3D011623D5C

namespace utils::detail
{
using ray_type = std::pair<Eigen::Vector3f, Eigen::Vector3f>;
using tri_type = std::tuple<Eigen::Vector3f, Eigen::Vector3f, Eigen::Vector3f>;
} // namespace utils::detail

namespace utils
{
struct RaycastResult final {
    Eigen::Vector3f hit_position;
    Eigen::Vector3f hit_normal;
    float hit_distance;
};
} // namespace utils

namespace utils
{
detail::ray_type unproject_ray(const Eigen::Matrix4f& inverse_view_projection) noexcept;
detail::ray_type unproject_ray(const Eigen::Matrix4f& inverse_view_projection, const Eigen::Vector2f& cursor_ndc) noexcept;
} // namespace utils

namespace utils
{
bool raycast_aabb(const detail::ray_type& ray, const Eigen::AlignedBox3f& aabb, utils::RaycastResult& result) noexcept;
bool raycast_triangle(const detail::ray_type& ray, const detail::tri_type& triangle, bool pick_backfaces,
    utils::RaycastResult& result) noexcept;
bool raycast_plane(const detail::ray_type& ray, const Eigen::Hyperplane<float, 3>& plane, utils::RaycastResult& result) noexcept;
} // namespace utils

#endif /* DBAF77A6_5F41_42F5_BA50_B3D011623D5C */
