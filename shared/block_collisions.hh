#ifndef B6A54337_7869_417A_8BB1_A42EB7E96B49
#define B6A54337_7869_417A_8BB1_A42EB7E96B49

#include "shared/block.hh"

struct BakedBlockCollision final {
    std::vector<Eigen::AlignedBox3f> elements;
    Eigen::AlignedBox3f bounds;
};

namespace block_collisions
{
void init_late(void);
void shutdown(void);
} // namespace block_collisions

namespace block_collisions
{
const BakedBlockCollision* find(block_id_type id);
} // namespace block_collisions

#endif /* B6A54337_7869_417A_8BB1_A42EB7E96B49 */
