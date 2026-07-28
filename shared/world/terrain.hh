#ifndef F85A4E6F_D512_4D91_8CDD_7F19E28AE056
#define F85A4E6F_D512_4D91_8CDD_7F19E28AE056

#include "shared/constant.hh"
#include "shared/coord.hh"

class BlockStorage;

namespace terrain
{
constexpr static BlockPos::value_type SURFACE_MIN_Y = -64;
constexpr static BlockPos::value_type SURFACE_MAX_Y = 255;
constexpr static BlockPos::value_type SKY_MIN_Y = 256;
constexpr static BlockPos::value_type SKY_MAX_Y = 767;
} // namespace terrain

namespace terrain
{
constexpr static ChunkPos::value_type SURFACE_MIN_CHUNK_Y = SURFACE_MIN_Y >> constant::CHUNK_SIZE_LOG2;
constexpr static ChunkPos::value_type SURFACE_MAX_CHUNK_Y = SURFACE_MAX_Y >> constant::CHUNK_SIZE_LOG2;
constexpr static ChunkPos::value_type SKY_MIN_CHUNK_Y = SKY_MIN_Y >> constant::CHUNK_SIZE_LOG2;
constexpr static ChunkPos::value_type SKY_MAX_CHUNK_Y = SKY_MAX_Y >> constant::CHUNK_SIZE_LOG2;
} // namespace terrain

namespace terrain
{
bool generate(const ChunkPos& pos, BlockStorage& storage);
} // namespace terrain

#endif /* F85A4E6F_D512_4D91_8CDD_7F19E28AE056 */
