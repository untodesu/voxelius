#include "core/pch.hh"

#include "core/utils/raycast.hh"

utils::detail::ray_type utils::unproject_ray(const Eigen::Matrix4f& inverse_view_projection) noexcept
{
    return unproject_ray(inverse_view_projection, Eigen::Vector2f::Zero());
}

utils::detail::ray_type utils::unproject_ray(const Eigen::Matrix4f& inverse_view_projection, const Eigen::Vector2f& cursor_ndc) noexcept
{
    Eigen::Vector4f plane_near(cursor_ndc.x(), cursor_ndc.y(), -1.0f, 1.0f);
    Eigen::Vector4f plane_far(cursor_ndc.x(), cursor_ndc.y(), 1.0f, 1.0f);

    Eigen::Vector4f world_near(inverse_view_projection * plane_near);
    Eigen::Vector4f world_far(inverse_view_projection * plane_far);

    world_near /= world_near.w();
    world_far /= world_far.w();

    Eigen::Vector3f origin(world_near.head<3>());
    Eigen::Vector3f direction((world_far.head<3>() - origin).normalized());
    return std::make_pair(origin, direction);
}

bool utils::raycast_aabb(const detail::ray_type& ray, const Eigen::AlignedBox3f& aabb, utils::RaycastResult& result) noexcept
{
    Eigen::Vector3f inverse_direction(ray.second.cwiseInverse());
    Eigen::Vector3f alpha_min((aabb.min() - ray.first).cwiseProduct(inverse_direction));
    Eigen::Vector3f alpha_max((aabb.max() - ray.first).cwiseProduct(inverse_direction));

    auto alpha_enter = alpha_min.cwiseMin(alpha_max).maxCoeff();
    auto alpha_exit = alpha_min.cwiseMax(alpha_max).minCoeff();

    if(alpha_exit < 0.0f || alpha_enter > alpha_exit) {
        return false; // no intersection
    }

    result.hit_distance = (alpha_enter >= 0.0f) ? alpha_enter : alpha_exit;
    result.hit_position = ray.first + result.hit_distance * ray.second;
    result.hit_normal = Eigen::Vector3f::Zero(); // TODO: compute normal

    return true;
}

bool utils::raycast_triangle(const detail::ray_type& ray, const detail::tri_type& triangle, bool pick_backfaces,
    utils::RaycastResult& result) noexcept
{
    constexpr static float epsilon = 10.0f * std::numeric_limits<float>::epsilon();

    const auto& v0 = std::get<0>(triangle);
    const auto& v1 = std::get<1>(triangle);
    const auto& v2 = std::get<2>(triangle);

    Eigen::Vector3f edge_1(v1 - v0);
    Eigen::Vector3f edge_2(v2 - v0);
    Eigen::Vector3f h(ray.second.cross(edge_2));

    auto a = edge_1.dot(h);

    if(pick_backfaces) {
        if(a > -epsilon && a < epsilon) {
            return false; // ray is parallel to triangle
        }
    }
    else {
        if(a < epsilon) {
            return false; // ray is parallel to triangle or hits backface
        }
    }

    auto f = 1.0f / a;
    auto s = ray.first - v0;
    auto u = f * s.dot(h);

    if(u < 0.0f || u > 1.0f) {
        return false; // intersection point is outside the triangle
    }

    auto q = s.cross(edge_1);
    auto v = f * ray.second.dot(q);

    if(v < 0.0f || u + v > 1.0f) {
        return false; // intersection point is outside the triangle
    }

    result.hit_distance = f * edge_2.dot(q);

    if(result.hit_distance >= epsilon) {
        result.hit_position = ray.first + result.hit_distance * ray.second;
        result.hit_normal = edge_1.cross(edge_2).normalized();
        return true;
    }

    return false; // intersection point is behind the ray origin
}

bool utils::raycast_plane(const detail::ray_type& ray, const Eigen::Hyperplane<float, 3>& plane, utils::RaycastResult& result) noexcept
{
    Eigen::Vector3f normal = plane.normal();
    auto denominator = normal.dot(ray.second);

    if(std::abs(denominator) < 10.0f * std::numeric_limits<float>::epsilon()) {
        return false; // ray is parallel to plane
    }

    auto alpha = -plane.signedDistance(ray.first) / denominator;

    if(alpha < 10.0f * std::numeric_limits<float>::epsilon()) {
        return false; // intersection is behind the ray origin
    }

    result.hit_distance = alpha;
    result.hit_position = ray.first + alpha * ray.second;

    if(denominator < 0.0f) {
        result.hit_normal = normal;
    }
    else {
        result.hit_normal = -normal;
    }

    return true;
}
