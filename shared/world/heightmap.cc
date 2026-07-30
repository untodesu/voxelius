#include "shared/pch.hh"

#include "shared/world/heightmap.hh"

#include "core/utils/modulo.hh"

#include "shared/world/realm_sky.hh"
#include "shared/world/realm_surface.hh"

constexpr static std::size_t REALM_COUNT = static_cast<std::size_t>(BIOME_REALM_COUNT);

static std::mutex s_mutex;
static std::array<emhash8::HashMap<ChunkPosXZ, ColumnSlice>, REALM_COUNT> s_cache;

static ColumnSlice init_slice(biome_realm realm, const ChunkPosXZ& pos)
{
    switch(realm) {
        case BIOME_REALM_SURFACE:
            return realm_surface::probe(pos);

        case BIOME_REALM_SKY:
            return realm_sky::probe(pos);
    }

    return {};
}

static const ColumnSlice* find_cached(biome_realm realm, const ChunkPosXZ& pos)
{
    std::scoped_lock lock(s_mutex);

    auto index = static_cast<std::size_t>(realm);
    assert(index < REALM_COUNT);

    auto& cache = s_cache.at(index);
    auto it = cache.find(pos);

    if(it == cache.cend()) {
        return nullptr;
    }

    return &it->second;
}

static const ColumnSlice* insert_cached(biome_realm realm, const ChunkPosXZ& pos, ColumnSlice slice)
{
    std::scoped_lock lock(s_mutex);

    auto index = static_cast<std::size_t>(realm);
    assert(index < REALM_COUNT);

    auto& cache = s_cache.at(index);
    auto it = cache.insert_or_assign(pos, std::move(slice));
    return &it.first->second;
}

void heightmap::purge(void)
{
    std::scoped_lock lock(s_mutex);

    for(auto& cache : s_cache) {
        cache.clear();
    }
}

const Column& heightmap::probe_slow(biome_realm realm, const BlockPosXZ& pos)
{
    ChunkPosXZ cpos;
    cpos[0] = static_cast<ChunkPosXZ::value_type>(pos[0] >> constant::CHUNK_SIZE_LOG2);
    cpos[1] = static_cast<ChunkPosXZ::value_type>(pos[1] >> constant::CHUNK_SIZE_LOG2);

    auto cached = find_cached(realm, cpos);

    if(cached == nullptr) {
        auto slice = init_slice(realm, cpos);
        cached = insert_cached(realm, cpos, std::move(slice));
        assert(cached);
    }

    LocalPosXZ lpos;
    lpos[0] = static_cast<LocalPosXZ::value_type>(utils::mod_signed<BlockPosXZ::value_type>(pos[0], constant::CHUNK_SIZE));
    lpos[1] = static_cast<LocalPosXZ::value_type>(utils::mod_signed<BlockPosXZ::value_type>(pos[1], constant::CHUNK_SIZE));

    auto index = static_cast<std::size_t>(lpos[0] + lpos[1] * constant::CHUNK_SIZE);
    return cached->at(index);
}

const ColumnSlice& heightmap::probe(biome_realm realm, const ChunkPosXZ& pos)
{
    auto cached = find_cached(realm, pos);

    if(cached == nullptr) {
        auto slice = init_slice(realm, pos);
        cached = insert_cached(realm, pos, std::move(slice));
        assert(cached);
    }

    return *cached;
}
