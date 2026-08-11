#include "server/pch.hh"

#include "server/world/chunk_loader.hh"

#include "core/buffer.hh"
#include "core/threading.hh"

#include "shared/globals.hh"
#include "shared/utils/biome.hh"
#include "shared/world/chunk.hh"
#include "shared/world/world.hh"

#include "server/constant.hh"
#include "server/universe.hh"
#include "server/world/worldgen.hh"
#include "server/world/zvox_file.hh"

constexpr static std::size_t MAX_REGIONS = 32;

using RegionPos = Eigen::Vector<ChunkPos::value_type, 3>;

struct RegionKey final {
    bool operator==(const RegionKey& other) const;

    biome_realm realm;
    RegionPos pos;
};

template<>
struct std::hash<RegionKey> final {
    std::size_t operator()(const RegionKey& key) const
    {
        std::size_t value = 0;
        value ^= key.pos.x() * 73856093;
        value ^= key.pos.y() * 19349663;
        value ^= key.pos.z() * 83492791;
        value ^= static_cast<std::size_t>(key.realm) * 2654435761;
        return value;
    }
};

struct RegionEntry final {
    explicit RegionEntry(std::filesystem::path path);

    std::mutex io_mutex;
    ZvoxFile region_file;
    std::chrono::steady_clock::time_point last_used;
};

RegionEntry::RegionEntry(std::filesystem::path path) : region_file(std::move(path))
{
    // empty
}

static std::shared_mutex s_cache_mutex;
static vx::hash_map<RegionKey, std::shared_ptr<RegionEntry>> s_cache;
static vx::hash_set<RegionPos> s_pending;

bool RegionKey::operator==(const RegionKey& other) const
{
    return realm == other.realm && pos == other.pos;
}

static RegionPos region_of(const ChunkPos& pos)
{
    RegionPos region;
    region.x() = pos.x() >> constant::REGION_SIZE_LOG2;
    region.y() = pos.y() >> constant::REGION_SIZE_LOG2;
    region.z() = pos.z() >> constant::REGION_SIZE_LOG2;
    return region;
}

static std::size_t region_slot(const ChunkPos& pos, const RegionPos& region)
{
    auto lx = static_cast<std::size_t>(pos.x() - (region.x() << constant::REGION_SIZE_LOG2));
    auto ly = static_cast<std::size_t>(pos.y() - (region.y() << constant::REGION_SIZE_LOG2));
    auto lz = static_cast<std::size_t>(pos.z() - (region.z() << constant::REGION_SIZE_LOG2));
    return (ly * constant::REGION_SIZE + lz) * constant::REGION_SIZE + lx;
}

static std::filesystem::path region_path(biome_realm realm, const RegionPos& region)
{
    auto ux = std::bit_cast<std::uint32_t>(region.x());
    auto uy = std::bit_cast<std::uint32_t>(region.y());
    auto uz = std::bit_cast<std::uint32_t>(region.z());
    auto name = std::format("R{}X{:08X}Y{:08X}Z{:08X}.zvox", static_cast<unsigned>(realm), ux, uy, uz);
    return universe::chunk_dir() / name;
}

static std::shared_ptr<RegionEntry> find_region(biome_realm realm, const RegionPos& pos)
{
    RegionKey key {};
    key.realm = realm;
    key.pos = pos;

    std::shared_lock lock(s_cache_mutex);

    auto it = s_cache.find(key);

    if(it == s_cache.cend()) {
        return nullptr;
    }

    return it->second;
}

static std::shared_ptr<RegionEntry> get_region(biome_realm realm, const RegionPos& pos)
{
    if(auto entry = find_region(realm, pos)) {
        entry->last_used = std::chrono::steady_clock::now();
        return entry;
    }

    RegionKey key {};
    key.realm = realm;
    key.pos = pos;

    std::unique_lock lock(s_cache_mutex);

    auto entry = std::make_shared<RegionEntry>(region_path(realm, pos));
    entry->last_used = std::chrono::steady_clock::now();

    s_cache.insert_or_assign(key, std::shared_ptr(entry));

    return entry;
}

static void evict_stale(void)
{
    std::unique_lock lock(s_cache_mutex);

    if(s_cache.size() > MAX_REGIONS) {
        std::vector<std::pair<RegionKey, std::shared_ptr<RegionEntry>>> entries;
        entries.reserve(s_cache.size());

        for(const auto& it : s_cache) {
            entries.emplace_back(it.first, it.second);
        }

        std::sort(entries.begin(), entries.end(), [](const auto& a, const auto& b) {
            return a.second->last_used < b.second->last_used;
        });

        for(const auto& [key, entry] : entries) {
            if(s_cache.size() <= MAX_REGIONS) {
                break;
            }

            if(entry.use_count() <= 2) {
                // Baseline while iterating this snapshot is 2 refs
                // (s_cache's own + this local copy); anything above
                // that means an in-flight task is still using it.
                s_cache.erase(key);
            }
        }
    }
}

static void write_chunk(const ChunkPos& pos, const BlockStorage& blocks)
{
    auto realm = utils::realm(pos.y());
    auto region = region_of(pos);
    auto slot = region_slot(pos, region);

    auto entry = get_region(realm, region);

    WriteBuffer buffer;
    BlockStorage::encode(blocks, buffer);

    std::scoped_lock io_lock(entry->io_mutex);
    entry->region_file.write_slot(slot, buffer);
}

class LoadTask final : public Task {
public:
    explicit LoadTask(ChunkPos pos);
    virtual ~LoadTask(void) override;
    virtual void process(void) override;
    virtual void finalize(void) override;

private:
    ChunkPos m_pos;
    BlockStorage m_blocks;
    bool m_found { false };
};

class SaveTask final : public Task {
public:
    explicit SaveTask(ChunkPos pos, BlockStorage blocks);
    virtual ~SaveTask(void) override = default;
    virtual void process(void) override;
    virtual void finalize(void) override;

private:
    ChunkPos m_pos;
    BlockStorage m_blocks;
};

LoadTask::LoadTask(ChunkPos pos) : m_pos(std::move(pos))
{
    m_blocks.fill(BLOCK_ID_NULL);
}

LoadTask::~LoadTask(void)
{
    s_pending.erase(m_pos);
}

void LoadTask::process(void)
{
    ZoneScopedN("chunk_loader::load");

    auto realm = utils::realm(m_pos.y());
    auto region = region_of(m_pos);
    auto slot = region_slot(m_pos, region);

    auto entry = get_region(realm, region);

    ReadBuffer buffer;

    std::scoped_lock io_lock(entry->io_mutex);

    if(entry->region_file.read_slot(slot, buffer)) {
        BlockStorage::decode(m_blocks, buffer);
        m_found = true;
    }
}

void LoadTask::finalize(void)
{
    ZoneScopedN("chunk_loader::load_finalize");

    if(m_found) {
        auto chunk = world::create_chunk(m_pos);
        chunk->set_blocks(std::move(m_blocks));

        // Not ChunkUpdateEvent: loaded-from-disk chunks are identical
        // to what's saved and must NOT be marked ChunkDirtyMarker.
        globals::dispatcher.trigger(ChunkReadyEvent(m_pos, chunk));
    }
    else {
        worldgen::request(m_pos);
    }
}

SaveTask::SaveTask(ChunkPos pos, BlockStorage blocks) : m_pos(std::move(pos)), m_blocks(std::move(blocks))
{
    // empty
}

void SaveTask::process(void)
{
    ZoneScopedN("chunk_loader::save");

    write_chunk(m_pos, m_blocks);
}

void SaveTask::finalize(void)
{
    // empty
}

static void on_chunk_update(const ChunkUpdateEvent& event)
{
    world::chunk_registry.emplace_or_replace<ChunkDirtyMarker>(event.chunk()->entity());
}

static void on_block_update(const BlockUpdateEvent& event)
{
    world::chunk_registry.emplace_or_replace<ChunkDirtyMarker>(event.chunk()->entity());
}

void chunk_loader::init(void)
{
    globals::dispatcher.sink<ChunkUpdateEvent>().connect<&on_chunk_update>();
    globals::dispatcher.sink<BlockUpdateEvent>().connect<&on_block_update>();
}

void chunk_loader::shutdown(void)
{
    auto view = world::chunk_registry.view<Chunk_Component, ChunkDirtyMarker>();

    for(auto [entity, component] : view.each()) {
        write_chunk(component.position, component.ptr->blocks());
    }

    std::unique_lock lock(s_cache_mutex);
    s_cache.clear();
}

void chunk_loader::fixed_update(void)
{
    evict_stale();
}

void chunk_loader::request(const ChunkPos& pos)
{
    if(world::find_chunk(pos)) {
        return;
    }

    if(s_pending.contains(pos)) {
        return;
    }

    s_pending.emplace(pos);

    threading::submit<LoadTask>(pos);
}

bool chunk_loader::pending(const ChunkPos& pos)
{
    return s_pending.contains(pos);
}

bool chunk_loader::save(const ChunkPos& pos)
{
    auto chunk = world::find_chunk(pos);

    if(chunk == nullptr) {
        return false;
    }

    auto entity = chunk->entity();

    if(world::chunk_registry.all_of<ChunkDirtyMarker>(entity)) {
        world::chunk_registry.remove<ChunkDirtyMarker>(entity);
        threading::submit<SaveTask>(pos, chunk->blocks());
        return true;
    }

    return false;
}
