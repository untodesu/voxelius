#ifndef C1DAD708_A904_4335_AD46_212640B0DC2B
#define C1DAD708_A904_4335_AD46_212640B0DC2B

#include "shared/constant.hh"
#include "shared/coord.hh"
#include "shared/world/biome.hh"

using heightmap_entry_type = std::array<BlockPos::value_type, constant::CHUNK_AREA>;

namespace heightmap
{
void purge(void);
} // namespace heightmap

namespace heightmap
{
void update(biome_realm realm, const ChunkPosXZ& pos, heightmap_entry_type entry);
} // namespace heightmap

namespace heightmap
{
const heightmap_entry_type& get(biome_realm realm, const ChunkPosXZ& pos);
} // namespace heightmap

#endif /* C1DAD708_A904_4335_AD46_212640B0DC2B */
