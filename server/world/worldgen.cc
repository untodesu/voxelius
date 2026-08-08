#include "server/pch.hh"

#include "server/world/worldgen.hh"

#include "core/threading.hh"

#include "shared/globals.hh"
#include "shared/utils/biome.hh"
#include "shared/world/biome_map.hh"
#include "shared/world/biome_slice.hh"
#include "shared/world/chunk.hh"
#include "shared/world/world.hh"

#include "server/world/climate_noise.hh"
#include "server/world/entropy_cache.hh"
#include "server/world/feature_placer.hh"
#include "server/world/feature_scatter.hh"
#include "server/world/heightmap.hh"
#include "server/world/realm_sky.hh"
#include "server/world/realm_surface.hh"
#include "server/world/terrain.hh"

static emhash8::HashMap<ChunkPos, std::nullptr_t> s_pending;

class WorldgenTask final : public Task {
public:
    explicit WorldgenTask(const ChunkPos& pos);
    virtual ~WorldgenTask(void) override;
    virtual void process(void) override;
    virtual void finalize(void) override;

private:
    BlockStorage m_blocks;
    std::unique_ptr<BiomeSlice> m_biomes;
    ChunkPos m_pos;
};

WorldgenTask::WorldgenTask(const ChunkPos& pos) : m_pos(pos)
{
    m_blocks.fill(BLOCK_ID_NULL);
}

WorldgenTask::~WorldgenTask(void)
{
    s_pending.erase(m_pos);
}

void WorldgenTask::process(void)
{
    ZoneScopedN("worldgen::process");

    auto realm = utils::realm(m_pos.y());

    if(realm != BIOME_REALM_SURFACE && realm != BIOME_REALM_SKY) {
        status.store(task_status::CANCELLED, std::memory_order_release);
        return;
    }

    auto biomes = std::make_unique<BiomeSlice>();

    if(!terrain::generate(m_pos, m_blocks, *biomes)) {
        status.store(task_status::CANCELLED, std::memory_order_release);
        return;
    }

    m_biomes = std::move(biomes);
}

void WorldgenTask::finalize(void)
{
    ZoneScopedN("worldgen::finalize");

    auto chunk = world::create_chunk(m_pos);
    chunk->set_blocks(std::move(m_blocks));

    if(m_biomes) {
        auto has_biome = std::any_of(m_biomes->cbegin(), m_biomes->cend(), [](biome_id_type id) {
            return static_cast<bool>(id);
        });

        if(has_biome) {
            auto realm = utils::realm(m_pos.y());
            auto cpos_xz = ChunkPosXZ(m_pos.x(), m_pos.z());
            biome_map::insert(realm, cpos_xz, std::move(*m_biomes));
        }

        m_biomes.reset();
    }

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
