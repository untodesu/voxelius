#include "shared/pch.hh"

#include "shared/world/terrain.hh"

#include "shared/constant.hh"
#include "shared/utils/biome.hh"
#include "shared/world/realm_sky.hh"
#include "shared/world/realm_surface.hh"

bool terrain::generate(const ChunkPos& pos, BlockStorage& blocks, BiomeSlice& biomes)
{
    auto realm = utils::realm(pos.y());
    auto generated = false;

    blocks.fill(BLOCK_ID_NULL);
    biomes.fill(BIOME_ID_NULL);

    switch(realm) {
        case BIOME_REALM_SURFACE:
            realm_surface::generate(blocks, biomes, pos);
            generated = true;
            break;

        case BIOME_REALM_SKY:
            realm_sky::generate(blocks, biomes, pos);
            generated = true;
            break;
    }

    // TODO: if generated, carve caves

    blocks.optimize();

    return generated;
}
