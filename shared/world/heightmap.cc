#include "shared/pch.hh"

#include "shared/world/heightmap.hh"

using heightmap_type = emhash8::HashMap<ChunkPosXZ, heightmap_entry_type>;

constexpr static std::size_t REALM_COUNT = static_cast<std::size_t>(BIOME_REALM_COUNT);

static std::mutex s_mutex;
static std::array<heightmap_type, REALM_COUNT> s_heightmaps;
static heightmap_entry_type s_empty_entry;
static std::once_flag s_empty_init;

void heightmap::purge(void)
{
    std::scoped_lock lock(s_mutex);

    for(auto& heightmap : s_heightmaps) {
        heightmap.clear();
    }
}

void heightmap::update(biome_realm realm, const ChunkPosXZ& pos, heightmap_entry_type entry)
{
    std::scoped_lock lock(s_mutex);

    auto index = static_cast<std::size_t>(realm);
    assert(index < REALM_COUNT);

    auto& heightmap = s_heightmaps.at(index);
    heightmap[pos] = std::move(entry);
}

const heightmap_entry_type& heightmap::get(biome_realm realm, const ChunkPosXZ& pos)
{
    auto index = static_cast<std::size_t>(realm);
    assert(index < REALM_COUNT);

    std::scoped_lock lock(s_mutex);

    std::call_once(s_empty_init, [] {
        s_empty_entry.fill(std::numeric_limits<BlockPos::value_type>::max());
    });

    auto& heightmap = s_heightmaps.at(index);
    auto it = heightmap.find(pos);

    if(it == heightmap.end()) {
        return s_empty_entry;
    }

    return it->second;
}
