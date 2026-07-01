#include "core/pch.hh"

#include "core/frustum.hh"

static Eigen::Hyperplane<float, 3> extract_hyperplane(const Eigen::Vector4f& coefficients)
{
    Eigen::Hyperplane<float, 3> result(coefficients.head<3>(), coefficients[3]);
    result.normalize();
    return result;
}

Frustum::Frustum(void)
{
    set_matrix(Eigen::Matrix4f::Identity());
}

Frustum::Frustum(const Eigen::Matrix4f& matrix)
{
    set_matrix(matrix);
}

void Frustum::set_plane(unsigned index, Eigen::Hyperplane<float, 3> plane) noexcept
{
    assert(index < m_planes.size());
    assert(plane.coeffs().allFinite());

    m_planes[index] = std::move(plane);
}

void Frustum::set_matrix(const Eigen::Matrix4f& matrix) noexcept
{
    assert(matrix.allFinite());

    Eigen::Vector4f row0(matrix.row(0));
    Eigen::Vector4f row1(matrix.row(1));
    Eigen::Vector4f row2(matrix.row(2));
    Eigen::Vector4f row3(matrix.row(3));

    m_planes[PLN_LF] = extract_hyperplane(row3 + row0);
    m_planes[PLN_RT] = extract_hyperplane(row3 - row0);
    m_planes[PLN_DN] = extract_hyperplane(row3 + row1);
    m_planes[PLN_UP] = extract_hyperplane(row3 - row1);
    m_planes[PLN_FT] = extract_hyperplane(row3 + row2);
    m_planes[PLN_BK] = extract_hyperplane(row3 - row2);
}

bool Frustum::contains(const Eigen::Vector3f& point) const noexcept
{
    for(const auto& hyperplane : m_planes) {
        if(hyperplane.signedDistance(point) < 0.0f) {
            return false;
        }
    }

    return true;
}

bool Frustum::intersects(const Eigen::AlignedBox3f& aabb) const noexcept
{
    Eigen::Vector3f center(aabb.center());
    Eigen::Vector3f extent(0.5f * aabb.sizes());

    for(const auto& hyperplane : m_planes) {
        auto r = extent.dot(hyperplane.normal().cwiseAbs());
        auto d = hyperplane.signedDistance(center);

        if(d < -r) {
            return false;
        }
    }

    return true;
}
