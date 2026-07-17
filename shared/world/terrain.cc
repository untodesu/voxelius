#include "shared/pch.hh"

#include "shared/world/terrain.hh"

#include "shared/utils/coord.hh"
#include "shared/world/block_registry.hh"
#include "shared/world/block_storage.hh"
#include "shared/world/noise_cache.hh"

static std::unique_ptr<NoiseCache_2D> s_test_cache;
static std::mutex s_init_mutex;

bool terrain::generate(const ChunkPos& pos, BlockStorage& storage)
{
    if(s_test_cache == nullptr) {
        std::scoped_lock lock(s_init_mutex);

        fnl_state noise = fnlCreateState();
        noise.noise_type = FNL_NOISE_OPENSIMPLEX2;
        noise.fractal_type = FNL_FRACTAL_FBM;
        noise.frequency = 0.01f;
        noise.octaves = 4;
        noise.lacunarity = 2.0f;
        noise.gain = 0.5f;

        s_test_cache = std::make_unique<NoiseCache_2D>(std::move(noise), Eigen::Vector2i(4, 4));
    }

    if(std::abs(pos.y()) > 2) {
        return false;
    }

    auto stone = block_registry::find(Identifier::from_string("builtin:stone"));

    for(std::size_t i = 0; i < constant::CHUNK_VOLUME; ++i) {
        auto lpos = utils::to_local(i);
        auto bpos = utils::to_block(pos, lpos);
        auto value = s_test_cache->sample(BlockPosXZ(bpos.x(), bpos.z()));
        auto value_scaled = static_cast<int>(value * static_cast<float>(constant::CHUNK_SIZE));

        if(bpos.y() < value_scaled) {
            storage.set(i, stone);
        }
    }

    return true;
}
