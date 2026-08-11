#ifndef B5D7F1CC_4793_44BF_919C_600F153A09DD
#define B5D7F1CC_4793_44BF_919C_600F153A09DD

#include "shared/coord.hh"

struct SimulatedMoveData final {
    ChunkPos chunk;
    Eigen::Vector3f local;
    Eigen::Vector3f velocity;
    Eigen::Vector3f angles;
};

namespace pmove_validator
{
void init(void);
void fixed_update_late(void);
} // namespace pmove_validator

#endif /* B5D7F1CC_4793_44BF_919C_600F153A09DD */
