#ifndef AA4B08C7_6967_49FB_885F_5CF3E078AE96
#define AA4B08C7_6967_49FB_885F_5CF3E078AE96

#include "shared/block.hh"
#include "shared/coord.hh"

namespace physics
{
enum block_filter {
    BLOCK_FILTER_NONE = 0, ///< Raycast will ignore all blocks
    BLOCK_FILTER_SOLID,    ///< Raycast will only hit solid blocks: must have a collision model and have a valid touch response
    BLOCK_FILTER_ALL,      ///< Raycast will hit all blocks with a collision model regardless of touch response
};

enum entity_filter {
    ENTITY_FILTER_NONE = 0, ///< Raycast will ignore all entities
    ENTITY_FILTER_ALL,      ///< Raycast will hit all entities
};
} // namespace physics

namespace physics
{
struct Ray final {
    ChunkPos start_chunk;
    Eigen::Vector3f start;
    Eigen::Vector3f direction;
    float max_distance;
};
} // namespace physics

namespace physics
{
struct Hit final {
    float distance;
    Eigen::Vector3f point; // float variant of local_pos
    Eigen::Vector3f normal;

    std::variant<block_id_type, entt::entity> target;

    BlockPos block_pos;
    ChunkPos chunk_pos;
    LocalPos local_pos;
};
} // namespace physics

namespace physics
{
std::optional<Hit> raycast_block(const Ray& ray, block_filter bfilter) noexcept;
std::optional<Hit> raycast_entity(const Ray& ray, entity_filter efilter) noexcept;
std::optional<Hit> raycast(const Ray& ray, block_filter bfilter, entity_filter efilter) noexcept;
} // namespace physics

#endif /* AA4B08C7_6967_49FB_885F_5CF3E078AE96 */
