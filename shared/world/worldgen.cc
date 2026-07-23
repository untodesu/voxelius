#include "shared/pch.hh"

#include "shared/world/worldgen.hh"

#include "core/threading.hh"

#include "shared/globals.hh"
#include "shared/world/chunk.hh"
#include "shared/world/entropy_cache.hh"
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
    ChunkPos m_pos;
};

WorldgenTask::WorldgenTask(const ChunkPos& pos) : m_pos(pos)
{
    m_blocks.fill(BLOCK_ID_NULL);
}

void WorldgenTask::process(void)
{
    if(!terrain::generate(m_pos, m_blocks)) {
        status.store(task_status::CANCELLED, std::memory_order_release);
    }
}

void WorldgenTask::finalize(void)
{
    auto chunk = world::create_chunk(m_pos);
    chunk->set_blocks(std::move(m_blocks));

    s_pending.erase(m_pos);

    globals::dispatcher.trigger(ChunkUpdateEvent(m_pos, chunk));
}

void worldgen::init(void)
{
    std::uint64_t seed = 1337;

    // TODO: pass in a world config / seed

    entropy_cache::init(seed);
    realm_surface::init(seed);
}

void worldgen::shutdown(void)
{
    entropy_cache::shutdown();
    realm_surface::shutdown();
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
