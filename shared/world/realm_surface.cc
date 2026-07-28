#include "shared/pch.hh"

#include "shared/world/realm_surface.hh"

#include "core/identifier.hh"

#include "shared/constant.hh"
#include "shared/utils/coord.hh"
#include "shared/world/block_registry.hh"
#include "shared/world/block_storage.hh"
#include "shared/world/climate.hh"
#include "shared/world/climate_noise.hh"
#include "shared/world/heightmap.hh"
#include "shared/world/noise_cache.hh"
#include "shared/world/terrain.hh"

struct SliceItem final {
    block_id_type palette_basic;
    block_id_type palette_filler;
    block_id_type palette_surface;
    block_id_type palette_fluid;
    float variation;
    float base;
};

constexpr static float VARIATION_MIN = 16.0f;
constexpr static float VARIATION_MAX = 96.0f;

constexpr static float BASE_MIN = -24.0f;
constexpr static float BASE_MAX = 16.0f;

constexpr static float PV_VALLEY_OFFSET = -8.0f;
constexpr static float PV_PEAK_OFFSET = 12.0f;

constexpr static LocalPos::value_type CHUNK_SIZE_LP = static_cast<LocalPos::value_type>(constant::CHUNK_SIZE);

static std::unique_ptr<NoiseCache_3D> s_terrain;

static bool is_inside_terrain(const BlockPos& bpos, float y_relative, float base_variation)
{
    auto variation_noise = s_terrain->sample(bpos);
    auto variation = base_variation * (1.0f - variation_noise * variation_noise);
    auto noise = variation * variation_noise - y_relative;

    return noise > 0.0f;
}

void realm_surface::init(std::mt19937_64& seeder)
{
    fnl_state noise_terrain = fnlCreateState();
    noise_terrain.seed = static_cast<int>(seeder());
    noise_terrain.noise_type = FNL_NOISE_OPENSIMPLEX2S;
    noise_terrain.fractal_type = FNL_FRACTAL_FBM;
    noise_terrain.frequency = 0.005f;
    noise_terrain.octaves = 4;

    s_terrain = std::make_unique<NoiseCache_3D>(std::move(noise_terrain), Eigen::Vector3i(4, 8, 4));
}

void realm_surface::shutdown(void)
{
    s_terrain.reset();
}

void realm_surface::generate(BlockStorage& storage, const ChunkPos& pos)
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
            item.variation = VARIATION_MIN;
            item.base = BASE_MIN;

            if(auto biome = climate::find(BIOME_REALM_SURFACE, sample)) {
                item.palette_basic = biome->palette_basic.cached;
                item.palette_filler = biome->palette_filler.cached;
                item.palette_surface = biome->palette_surface.cached;
                item.palette_fluid = biome->palette_fluid.cached;
            }

            auto continentalness = climate::normalize_01(sample.continentalness);
            auto erosion = climate::normalize_01(sample.erosion);
            auto pv = climate::peaks_valleys(sample.weirdness);

            item.variation = std::lerp(VARIATION_MAX, VARIATION_MIN, erosion);
            item.base = std::lerp(BASE_MIN, BASE_MAX, continentalness);
            item.base += std::lerp(PV_VALLEY_OFFSET, PV_PEAK_OFFSET, climate::normalize_01(pv));

            slice[index] = std::move(item);
        }
    }

    // Pass 1: density basic terrain fill
    for(std::size_t i = 0; i < constant::CHUNK_VOLUME; ++i) {
        auto lpos = utils::to_local(i);
        auto bpos = utils::to_block(pos, lpos);
        auto index_xz = static_cast<std::size_t>(lpos.x() + lpos.z() * constant::CHUNK_SIZE);

        auto& item = slice[index_xz];
        auto y_relative = static_cast<float>(bpos.y()) - item.base;
        auto has_terrain = false;

        if(y_relative > item.variation) {
            continue;
        }

        if(y_relative < -item.variation) {
            storage.set(i, item.palette_basic);
        }
        else if(is_inside_terrain(bpos, y_relative, item.variation)) {
            storage.set(i, item.palette_basic);
        }
        else if(bpos.y() < 0) {
            storage.set(i, item.palette_fluid);
        }
    }

    // Pass 2: surface skin
    for(std::size_t i = 0; i < constant::CHUNK_SIZE; ++i) {
        auto lpos = utils::to_local(i);
        auto bpos = utils::to_block(pos, lpos);
        auto index_xz = static_cast<std::size_t>(lpos.x() + lpos.z() * constant::CHUNK_SIZE);

        auto& item = slice[index_xz];
        auto y_relative = static_cast<float>(bpos.y()) - item.base;

        if(y_relative > item.variation) {
            continue;
        }

        auto current = storage.get(i);

        if(current == BLOCK_ID_NULL || current == item.palette_fluid) {
            continue;
        }

        auto depth = 0U;
        auto underwater = false;

        for(unsigned dy = 0; dy < 5; dy += 1) {
            auto d_lpos = LocalPos(lpos.x(), lpos.y() + static_cast<LocalPos::value_type>(dy + 1), lpos.z());
            auto d_bpos = utils::to_block(pos, d_lpos);
            auto d_index = utils::to_index(d_lpos);

            if(d_lpos.y() >= CHUNK_SIZE_LP) {
                auto y_relative = static_cast<float>(d_bpos.y()) - item.base;

                if(!is_inside_terrain(d_bpos, y_relative, item.variation)) {
                    underwater = dy == 0 && d_bpos.y() < 0;
                    break;
                }

                depth += 1U;
            }
            else {
                auto above = storage.get(d_index);

                if(above == BLOCK_ID_NULL || above == item.palette_fluid) {
                    underwater = dy == 0 && above == item.palette_fluid;
                    break;
                }

                depth += 1U;
            }
        }

        if(depth < 5) {
            if(underwater || depth > 0) {
                storage.set(i, item.palette_filler);
            }
            else {
                storage.set(i, item.palette_surface);
            }
        }
    }

    storage.optimize();

    auto cpos_xz = ChunkPosXZ(pos.x(), pos.z());
    auto heights = heightmap::get(BIOME_REALM_SURFACE, cpos_xz);

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

    heightmap::update(BIOME_REALM_SURFACE, cpos_xz, heights);
}
