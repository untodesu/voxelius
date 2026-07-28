#include "shared/pch.hh"

#include "shared/world/realm_sky.hh"

#include "shared/constant.hh"
#include "shared/utils/coord.hh"
#include "shared/world/block_storage.hh"
#include "shared/world/climate.hh"
#include "shared/world/climate_noise.hh"
#include "shared/world/noise_cache.hh"

void realm_sky::init(std::uint64_t seed)
{
    // empty
}

void realm_sky::shutdown(void)
{
    // empty
}

bool realm_sky::generate(BlockStorage& storage, const ChunkPos& pos)
{
    return false;
}
