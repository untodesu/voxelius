#ifndef AB9A226A_0868_4978_BF1B_14ED1B503A24
#define AB9A226A_0868_4978_BF1B_14ED1B503A24

#include "shared/block.hh"
#include "shared/coord.hh"

class Chunk;

class Ray_DDA final {
public:
    Ray_DDA(void) noexcept = default;
    explicit Ray_DDA(const ChunkPos& start_chunk, const Eigen::Vector3f& start_pos, const Eigen::Vector3f& direction) noexcept;

    void reset(const ChunkPos& start_chunk, const Eigen::Vector3f& start_pos, const Eigen::Vector3f& direction) noexcept;

    block_id_type step(void) noexcept;

    constexpr const ChunkPos& start_chunk(void) const noexcept;
    constexpr const Eigen::Vector3f& start_pos(void) const noexcept;
    constexpr const Eigen::Vector3f& direction(void) const noexcept;

    constexpr const Eigen::Vector3f& delta_distance(void) const noexcept;
    constexpr const Eigen::Vector3f& side_distance(void) const noexcept;
    constexpr const BlockPos& block_step(void) const noexcept;

    constexpr float distance(void) const noexcept;
    constexpr const BlockPos& position(void) const noexcept;
    constexpr const BlockPos& normal(void) const noexcept;

private:
    ChunkPos m_start_chunk;
    Eigen::Vector3f m_start_pos;
    Eigen::Vector3f m_direction;

    Eigen::Vector3f m_delta_distance;
    Eigen::Vector3f m_side_distance;
    BlockPos m_block_step;

    float m_distance;
    BlockPos m_position;
    BlockPos m_normal;

    ChunkPos m_last_chunk_pos;
    std::shared_ptr<Chunk> m_last_chunk;
};

constexpr const ChunkPos& Ray_DDA::start_chunk(void) const noexcept
{
    return m_start_chunk;
}

constexpr const Eigen::Vector3f& Ray_DDA::start_pos(void) const noexcept
{
    return m_start_pos;
}

constexpr const Eigen::Vector3f& Ray_DDA::direction(void) const noexcept
{
    return m_direction;
}

constexpr const Eigen::Vector3f& Ray_DDA::delta_distance(void) const noexcept
{
    return m_delta_distance;
}

constexpr const Eigen::Vector3f& Ray_DDA::side_distance(void) const noexcept
{
    return m_side_distance;
}

constexpr const BlockPos& Ray_DDA::block_step(void) const noexcept
{
    return m_block_step;
}

constexpr float Ray_DDA::distance(void) const noexcept
{
    return m_distance;
}

constexpr const BlockPos& Ray_DDA::position(void) const noexcept
{
    return m_position;
}

constexpr const BlockPos& Ray_DDA::normal(void) const noexcept
{
    return m_normal;
}

#endif /* AB9A226A_0868_4978_BF1B_14ED1B503A24 */
