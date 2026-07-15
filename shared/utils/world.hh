#ifndef C52EB39B_B4F6_4E65_A3E6_F664EEC9C8DE
#define C52EB39B_B4F6_4E65_A3E6_F664EEC9C8DE

#include "shared/block.hh"
#include "shared/coord.hh"

namespace physics
{
struct BlockHit;
} // namespace physics

namespace utils
{
bool block_place(const physics::BlockHit& hit, entt::entity actor, block_id_type id);
bool block_break(const physics::BlockHit& hit, entt::entity actor);
bool block_interact(const physics::BlockHit& hit, entt::entity actor);
bool block_random_tick(const BlockPos& pos);
bool block_sched_tick(const BlockPos& pos);
} // namespace utils

#endif /* C52EB39B_B4F6_4E65_A3E6_F664EEC9C8DE */
