#include "shared/pch.hh"

#include "shared/world/terrain.hh"

#include "shared/constant.hh"
#include "shared/utils/coord.hh"
#include "shared/world/block_storage.hh"
#include "shared/world/realm_surface.hh"

constexpr static BlockPos::value_type SURFACE_MIN_Y = -64;
constexpr static BlockPos::value_type SURFACE_MAX_Y = 255;

constexpr static ChunkPos::value_type SURFACE_MIN_CHUNK_Y = SURFACE_MIN_Y >> constant::CHUNK_SIZE_LOG2;
constexpr static ChunkPos::value_type SURFACE_MAX_CHUNK_Y = SURFACE_MAX_Y >> constant::CHUNK_SIZE_LOG2;

bool terrain::generate(const ChunkPos& pos, BlockStorage& storage)
{
    if(pos.y() >= SURFACE_MIN_CHUNK_Y && pos.y() <= SURFACE_MAX_CHUNK_Y) {
        return realm_surface::generate(storage, pos);
    }

    return false;
}
