#include "shared/pch.hh"

#include "shared/world/terrain.hh"

#include "shared/world/block_registry.hh"
#include "shared/world/block_storage.hh"

bool terrain::generate(const ChunkPos& pos, BlockStorage& storage)
{
    if(pos.y() < 0) {
        auto stone = block_registry::find(Identifier::from_string("builtin:stone"));
        storage.fill(stone);
        return true;
    }

    return false;
}
