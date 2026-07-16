#ifndef E58051D6_68EC_495E_92C9_545EBD9CD3E9
#define E58051D6_68EC_495E_92C9_545EBD9CD3E9

#include "shared/world/block.hh"

struct BakedBlockModel_Quad final {
    std::array<Eigen::Vector3f, 4> positions;
    std::array<Eigen::Vector2f, 4> uvs;
    std::uint32_t packed_normal;
    std::uint32_t texture_index;
    std::size_t frame_count;
    unsigned tint_index;
    bool animated;
    bool shade;
};

struct BakedBlockModel final {
    std::vector<BakedBlockModel_Quad> unculled_quads;
    std::array<std::vector<BakedBlockModel_Quad>, 6> face_quads;
    std::array<bool, 6> fully_covered;
};

namespace block_models
{
void init_late(void);
void shutdown(void);
} // namespace block_models

namespace block_models
{
const BakedBlockModel* find(block_id_type id);
} // namespace block_models

#endif /* E58051D6_68EC_495E_92C9_545EBD9CD3E9 */
