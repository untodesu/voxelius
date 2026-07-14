#ifndef DD1519F9_BFB2_4D8F_A405_C48ECC5EEF9D
#define DD1519F9_BFB2_4D8F_A405_C48ECC5EEF9D

class Ray_AABB final {
public:
    struct Hit final {
        Eigen::Vector3f point;
        Eigen::Vector3f normal;
        float distance;
    };

    Ray_AABB(void) noexcept = default;

    explicit Ray_AABB(const Eigen::Vector3f& start, const Eigen::Vector3f& direction) noexcept;

    void reset(const Eigen::Vector3f& start, const Eigen::Vector3f& direction) noexcept;

    std::optional<Hit> intersect(const Eigen::AlignedBox3f& aabb) const noexcept;

    constexpr const Eigen::Vector3f& start(void) const noexcept;
    constexpr const Eigen::Vector3f& direction(void) const noexcept;
    constexpr const Eigen::Vector3f& inverse_direction(void) const noexcept;

private:
    Eigen::Vector3f m_start;
    Eigen::Vector3f m_direction;
    Eigen::Vector3f m_inverse_direction;
};

constexpr const Eigen::Vector3f& Ray_AABB::start(void) const noexcept
{
    return m_start;
}

constexpr const Eigen::Vector3f& Ray_AABB::direction(void) const noexcept
{
    return m_direction;
}

constexpr const Eigen::Vector3f& Ray_AABB::inverse_direction(void) const noexcept
{
    return m_inverse_direction;
}

#endif /* DD1519F9_BFB2_4D8F_A405_C48ECC5EEF9D */
