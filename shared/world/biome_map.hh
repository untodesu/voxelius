#ifndef B0F02F57_26DF_495B_8E6D_A132AAC33F58
#define B0F02F57_26DF_495B_8E6D_A132AAC33F58

#include "shared/coord.hh"
#include "shared/world/biome_slice.hh"

namespace biome_map
{
void init(void);
void purge(void);
void insert(biome_realm realm, const ChunkPosXZ& pos, BiomeSlice biomes);
} // namespace biome_map

namespace biome_map
{
biome_id_type get_slow(const BlockPos& pos);
biome_id_type get_slow(biome_realm realm, const BlockPosXZ& pos);
const BiomeSlice& get(const ChunkPos& pos);
const BiomeSlice& get(biome_realm realm, const ChunkPosXZ& pos);
} // namespace biome_map

#endif /* B0F02F57_26DF_495B_8E6D_A132AAC33F58 */
