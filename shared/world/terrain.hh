#ifndef F85A4E6F_D512_4D91_8CDD_7F19E28AE056
#define F85A4E6F_D512_4D91_8CDD_7F19E28AE056

#include "shared/constant.hh"
#include "shared/coord.hh"
#include "shared/world/biome_storage.hh"
#include "shared/world/block_storage.hh"

namespace terrain
{
bool generate(const ChunkPos& pos, BlockStorage& blocks, BiomeStorage::array_type& biomes);
} // namespace terrain

#endif /* F85A4E6F_D512_4D91_8CDD_7F19E28AE056 */
