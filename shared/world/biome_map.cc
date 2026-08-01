#include "shared/pch.hh"

#include "shared/world/biome_map.hh"

#include "shared/globals.hh"
#include "shared/utils/biome.hh"
#include "shared/utils/coord.hh"
#include "shared/world/world.hh"

struct MapEntry final {
    std::shared_ptr<BiomeSlice> ptr;
    bool initialized { false };
};

static std::mutex s_mutex;
static std::array<emhash8::HashMap<ChunkPosXZ, MapEntry>, NUM_BIOME_REALMS> s_biomes;
static BiomeSlice s_empty_slice {};
static std::shared_ptr<BiomeSlice> s_void_slice;

static ChunkPosXZ to_column(const ChunkPos& cpos)
{
    return ChunkPosXZ(cpos.x(), cpos.z());
}

static void on_chunk_create(const ChunkCreateEvent& event)
{
    auto& chunk = event.chunk();
    auto entity = chunk->entity();
    auto& cpos = event.pos();
    auto realm = utils::realm(cpos.y());

    BiomeSlice_Component component {};

    if(realm == BIOME_REALM_VOID) {
        component.ptr = s_void_slice;
        world::chunk_entities.emplace<BiomeSlice_Component>(entity, std::move(component));
        return;
    }

    auto index = static_cast<std::size_t>(realm);
    assert(index < NUM_BIOME_REALMS);

    auto cpos_xz = to_column(cpos);

    std::scoped_lock lock(s_mutex);

    auto& map = s_biomes[index];
    auto it = map.find(cpos_xz);

    if(it == map.end()) {
        component.ptr = std::make_shared<BiomeSlice>();

        MapEntry entry {};
        entry.ptr = component.ptr;
        entry.initialized = false;

        map.insert_or_assign(cpos_xz, std::move(entry));
    }
    else {
        component.ptr = it->second.ptr;
    }

    world::chunk_entities.emplace<BiomeSlice_Component>(entity, std::move(component));
}

void biome_map::init(void)
{
    s_void_slice = std::make_shared<BiomeSlice>();
    globals::dispatcher.sink<ChunkCreateEvent>().connect<&on_chunk_create>();
}

void biome_map::purge(void)
{
    std::scoped_lock lock(s_mutex);

    for(auto& realm_map : s_biomes) {
        realm_map.clear();
    }
}

void biome_map::insert(biome_realm realm, const ChunkPosXZ& pos, BiomeSlice biomes)
{
    if(realm == BIOME_REALM_VOID) {
        return;
    }

    auto index = static_cast<std::size_t>(realm);
    assert(index < NUM_BIOME_REALMS);

    std::scoped_lock lock(s_mutex);

    auto& map = s_biomes[index];
    auto it = map.find(pos);

    if(it == map.end()) {
        MapEntry entry {};
        entry.ptr = std::make_shared<BiomeSlice>(std::move(biomes));
        entry.initialized = true;
        map.insert_or_assign(pos, std::move(entry));
        return;
    }

    // First successful write wins for the realm column.
    if(it->second.initialized) {
        return;
    }

    *it->second.ptr = std::move(biomes);
    it->second.initialized = true;
}

biome_id_type biome_map::get_slow(const BlockPos& pos)
{
    auto pos_xz = BlockPosXZ(pos.x(), pos.z());
    auto cpos = utils::to_chunk(pos);
    auto realm = utils::realm(cpos.y());
    return get_slow(realm, pos_xz);
}

biome_id_type biome_map::get_slow(biome_realm realm, const BlockPosXZ& pos)
{
    if(realm == BIOME_REALM_VOID) {
        return BIOME_ID_NULL;
    }

    auto cpos_xz = utils::to_chunk_xz(pos);
    auto lpos_xz = utils::to_local_xz(pos);
    auto index_xz = utils::to_index_xz(lpos_xz);

    auto index = static_cast<std::size_t>(realm);
    assert(index < NUM_BIOME_REALMS);

    std::scoped_lock lock(s_mutex);

    auto& map = s_biomes[index];
    auto it = map.find(cpos_xz);

    if(it == map.end()) {
        return BIOME_ID_NULL;
    }

    return (*it->second.ptr)[index_xz];
}

const BiomeSlice& biome_map::get(const ChunkPos& pos)
{
    auto realm = utils::realm(pos.y());
    return get(realm, to_column(pos));
}

const BiomeSlice& biome_map::get(biome_realm realm, const ChunkPosXZ& pos)
{
    if(realm == BIOME_REALM_VOID) {
        return s_empty_slice;
    }

    auto index = static_cast<std::size_t>(realm);
    assert(index < NUM_BIOME_REALMS);

    std::scoped_lock lock(s_mutex);

    auto& map = s_biomes[index];
    auto it = map.find(pos);

    if(it == map.end()) {
        return s_empty_slice;
    }

    return *it->second.ptr;
}
