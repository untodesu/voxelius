#include "shared/pch.hh"

#include "shared/world/realm_sky.hh"

#include "shared/constant.hh"
#include "shared/utils/coord.hh"
#include "shared/world/block_storage.hh"
#include "shared/world/climate.hh"
#include "shared/world/climate_noise.hh"
#include "shared/world/heightmap.hh"
#include "shared/world/noise_cache.hh"

struct SliceItem final {
    block_id_type palette_basic;
    block_id_type palette_filler;
    block_id_type palette_surface;
    block_id_type palette_fluid;
    float continentalness;
};

constexpr static float BASE_CENTER_Y = 320.0f;
constexpr static float MAX_Y_BIAS = 96.0f;

static std::unique_ptr<NoiseCache_3D> s_density;

static bool is_inside_sky_terrain(const BlockPos& bpos, float continentalness)
{
    auto center_y = BASE_CENTER_Y + MAX_Y_BIAS * continentalness;
    auto relative_y = static_cast<float>(bpos.y()) - center_y;
    auto density = s_density->sample(bpos);

    if(relative_y > 0.0f) {
        density -= relative_y * 0.05f;
    }
    else {
        density -= std::abs(relative_y) * 0.015f;
    }

    return density > 0.0f;
}

void realm_sky::init(std::mt19937_64& seeder)
{
    auto density = fnlCreateState();
    density.seed = static_cast<int>(seeder());
    density.noise_type = FNL_NOISE_OPENSIMPLEX2S;
    density.fractal_type = FNL_FRACTAL_FBM;
    density.frequency = 0.005f;
    density.octaves = 4;

    s_density = std::make_unique<NoiseCache_3D>(std::move(density), Eigen::Vector3i(4, 8, 4));
}

void realm_sky::shutdown(void)
{
    s_density.reset();
}

void realm_sky::generate(BlockStorage& storage, const ChunkPos& pos)
{
    thread_local std::array<SliceItem, constant::CHUNK_VOLUME> slice;

    storage.fill(BLOCK_ID_NULL);

    for(LocalPos::value_type lz = 0; lz < constant::CHUNK_SIZE; lz += 1) {
        for(LocalPos::value_type lx = 0; lx < constant::CHUNK_SIZE; lx += 1) {
            auto index = static_cast<std::size_t>(lx + lz * constant::CHUNK_SIZE);
            auto bpos = utils::to_block(pos, LocalPos(lx, 0, lz));
            auto bpos_xz = BlockPosXZ(bpos.x(), bpos.z());

            auto sample = climate_noise::sample(bpos_xz);

            SliceItem item {};
            item.palette_basic = BLOCK_ID_NULL;
            item.palette_filler = BLOCK_ID_NULL;
            item.palette_surface = BLOCK_ID_NULL;
            item.palette_fluid = BLOCK_ID_NULL;

            if(auto biome = climate::find(BIOME_REALM_SKY, sample)) {
                item.palette_basic = biome->palette_basic.cached;
                item.palette_filler = biome->palette_filler.cached;
                item.palette_surface = biome->palette_surface.cached;
                item.palette_fluid = biome->palette_fluid.cached;
            }

            item.continentalness = climate::normalize_01(sample.continentalness);

            slice[index] = std::move(item);
        }
    }

    // Pass 1: density basic terrain fill
    for(std::size_t i = 0; i < constant::CHUNK_VOLUME; ++i) {
        auto lpos = utils::to_local(i);
        auto bpos = utils::to_block(pos, lpos);
        auto index_xz = static_cast<std::size_t>(lpos.x() + lpos.z() * constant::CHUNK_SIZE);

        auto& item = slice[index_xz];

        if(is_inside_sky_terrain(bpos, item.continentalness)) {
            storage.set(i, item.palette_basic);
        }
    }

    // Pass 2: surface skin
    for(std::size_t i = 0; i < constant::CHUNK_VOLUME; ++i) {
        auto lpos = utils::to_local(i);
        auto bpos = utils::to_block(pos, lpos);
        auto index_xz = static_cast<std::size_t>(lpos.x() + lpos.z() * constant::CHUNK_SIZE);

        auto& item = slice[index_xz];
        auto current = storage.get(i);

        if(current == BLOCK_ID_NULL || current == item.palette_fluid) {
            continue;
        }

        auto depth = 0U;

        for(unsigned dy = 0; dy < 5; dy += 1) {
            auto d_lpos = LocalPos(lpos.x(), lpos.y() + static_cast<LocalPos::value_type>(dy + 1), lpos.z());
            auto d_bpos = utils::to_block(pos, d_lpos);

            if(!is_inside_sky_terrain(d_bpos, item.continentalness)) {
                break;
            }

            depth += 1U;
        }

        if(depth < 5) {
            if(depth > 0) {
                storage.set(i, item.palette_filler);
            }
            else {
                storage.set(i, item.palette_surface);
            }
        }
    }

    storage.optimize();

    auto cpos_xz = ChunkPosXZ(pos.x(), pos.z());
    auto heights = heightmap::get(BIOME_REALM_SKY, cpos_xz);

    for(std::size_t i = 0; i < constant::CHUNK_VOLUME; ++i) {
        auto lpos = utils::to_local(i);
        auto bpos = utils::to_block(pos, lpos);
        auto index_xz = static_cast<std::size_t>(lpos.x() + lpos.z() * constant::CHUNK_SIZE);

        auto& item = slice[index_xz];
        auto current = storage.get(i);

        if(current == BLOCK_ID_NULL || current == item.palette_fluid) {
            continue;
        }

        if(bpos.y() > heights[index_xz]) {
            heights[index_xz] = bpos.y();
        }
    }

    heightmap::update(BIOME_REALM_SKY, cpos_xz, heights);
}
