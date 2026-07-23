#ifndef D4552A85_A3CB_44D1_903D_7008C2667C84
#define D4552A85_A3CB_44D1_903D_7008C2667C84

#include "shared/constant.hh"
#include "shared/coord.hh"

using cached_entropy_type = std::array<std::uint64_t, constant::CHUNK_AREA>;

namespace entropy_cache
{
void init(std::uint64_t seed);
void shutdown(void);
} // namespace entropy_cache

namespace entropy_cache
{
const cached_entropy_type& get(const ChunkPosXZ& pos);
} // namespace entropy_cache

#endif /* D4552A85_A3CB_44D1_903D_7008C2667C84 */
