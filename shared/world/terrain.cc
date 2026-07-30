#include "shared/pch.hh"

#include "shared/world/terrain.hh"

#include "shared/constant.hh"
#include "shared/utils/coord.hh"
#include "shared/world/block_storage.hh"
#include "shared/world/realm_sky.hh"
#include "shared/world/realm_surface.hh"

static bool check_range(ChunkPos::value_type y, ChunkPos::value_type min, ChunkPos::value_type max)
{
    return y >= min && y <= max;
}

bool terrain::generate(const ChunkPos& pos, BlockStorage& storage)
{
    auto generated = false;
    auto vertical = pos.y();

    storage.fill(BLOCK_ID_NULL);

    if(check_range(vertical, SURFACE_MIN_CHUNK_Y, SURFACE_MAX_CHUNK_Y)) {
        realm_surface::generate(storage, pos);
        generated = true;
    }
    else if(check_range(vertical, SKY_MIN_CHUNK_Y, SKY_MAX_CHUNK_Y)) {
        realm_sky::generate(storage, pos);
        generated = true;
    }

    // TODO: if generated, carve caves

    storage.optimize();

    return generated;
}
