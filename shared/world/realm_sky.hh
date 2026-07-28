#ifndef CDF3046F_AD1A_4954_8B4A_00B7AFB2A2B0
#define CDF3046F_AD1A_4954_8B4A_00B7AFB2A2B0

#include "shared/coord.hh"

class BlockStorage;

namespace realm_sky
{
void init(std::uint64_t seed);
void shutdown(void);
} // namespace realm_sky

namespace realm_sky
{
bool generate(BlockStorage& storage, const ChunkPos& pos);
} // namespace realm_sky

#endif /* CDF3046F_AD1A_4954_8B4A_00B7AFB2A2B0 */
