#include "shared/pch.hh"

#include "shared/world/worldgen.hh"

#include "core/threading.hh"

#include "shared/globals.hh"
#include "shared/world/chunk.hh"
#include "shared/world/climate_noise.hh"
#include "shared/world/entropy_cache.hh"
#include "shared/world/feature_placer.hh"
#include "shared/world/feature_scatter.hh"
#include "shared/world/heightmap.hh"
#include "shared/world/realm_sky.hh"
#include "shared/world/realm_surface.hh"
#include "shared/world/terrain.hh"
#include "shared/world/world.hh"

static emhash8::HashMap<ChunkPos, std::nullptr_t> s_pending;

class WorldgenTask final : public Task {
public:
    explicit WorldgenTask(const ChunkPos& pos);
    virtual ~WorldgenTask(void) = default;
    virtual void process(void) override;
    virtual void finalize(void) override;

private:
    BlockStorage m_blocks;
    BiomeStorage::array_type m_biomes;
    ChunkPos m_pos;
};

WorldgenTask::WorldgenTask(const ChunkPos& pos) : m_pos(pos)
{
    m_blocks.fill(BLOCK_ID_NULL);
}

void WorldgenTask::process(void)
{
    ZoneScopedN("worldgen::process");

    if(!terrain::generate(m_pos, m_blocks, m_biomes)) {
        status.store(task_status::CANCELLED, std::memory_order_release);
    }
}

void WorldgenTask::finalize(void)
{
    ZoneScopedN("worldgen::finalize");

    auto chunk = world::create_chunk(m_pos);
    auto& biomes = chunk->biomes();

    chunk->set_blocks(std::move(m_blocks));

    // Biome lookups are immutable per realm column
    // so it makes sense to only submit this data once
    std::call_once(biomes->initialized(), [&] {
        biomes->set_biomes(std::move(m_biomes));
    });

    s_pending.erase(m_pos);

    globals::dispatcher.trigger(ChunkUpdateEvent(m_pos, chunk));
}

void worldgen::init(void)
{
    std::uint64_t seed = 1337;
    std::mt19937_64 seeder(seed);

    // TODO: pass in a world config / seed

    entropy_cache::init(seeder);
    climate_noise::init(seeder);

    realm_surface::init(seeder);
    realm_sky::init(seeder);

    feature_placer::init();

    FeatureScatter::init();
}

void worldgen::shutdown(void)
{
    feature_placer::shutdown();

    realm_sky::shutdown();
    realm_surface::shutdown();

    climate_noise::shutdown();
    heightmap::purge();
    entropy_cache::shutdown();
}

void worldgen::request(const ChunkPos& pos)
{
    if(s_pending.contains(pos)) {
        return;
    }

    s_pending.emplace(pos, nullptr);

    threading::submit<WorldgenTask>(pos);
}

bool worldgen::pending(const ChunkPos& pos)
{
    return s_pending.contains(pos);
}
