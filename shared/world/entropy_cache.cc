#include "shared/pch.hh"

#include "shared/world/entropy_cache.hh"

struct Entry final {
    std::once_flag init_flag;
    cached_entropy_type data;
};

static std::mutex s_mutex;
static emhash8::HashMap<ChunkPosXZ, std::shared_ptr<Entry>> s_cache;
static std::uint64_t s_seed;

static void generate(const ChunkPosXZ& pos, cached_entropy_type& data)
{
    std::mt19937_64 random;
    random.seed(s_seed ^ std::hash<ChunkPosXZ>()(pos));

    for(std::size_t i = 0; i < constant::CHUNK_AREA; ++i) {
        data[i] = random();
    }
}

static std::shared_ptr<Entry> get_or_create(const ChunkPosXZ& pos)
{
    std::scoped_lock lock(s_mutex);

    auto it = s_cache.find(pos);

    if(it == s_cache.end()) {
        auto entry = std::make_shared<Entry>();
        s_cache[pos] = entry;
        return entry;
    }

    return it->second;
}

void entropy_cache::init(std::uint64_t seed)
{
    s_seed = seed;
}

void entropy_cache::shutdown(void)
{
    s_cache.clear();
}

const cached_entropy_type& entropy_cache::get(const ChunkPosXZ& pos)
{
    auto entry = get_or_create(pos);

    std::call_once(entry->init_flag, [pos, entry] {
        generate(pos, entry->data);
    });

    return entry->data;
}
