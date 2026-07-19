#include "shared/pch.hh"

#include "shared/world/realm_surface.hh"

#include "core/identifier.hh"

#include "shared/constant.hh"
#include "shared/utils/coord.hh"
#include "shared/world/block_registry.hh"
#include "shared/world/block_storage.hh"
#include "shared/world/climate.hh"
#include "shared/world/noise_cache.hh"

constexpr static float VARIATION_MIN = 16.0f;
constexpr static float VARIATION_MAX = 96.0f;
constexpr static float BASE_MIN = -24.0f;
constexpr static float BASE_MAX = 16.0f;

constexpr static BlockPos::value_type SEA_LEVEL = 0;

static std::unique_ptr<NoiseCache_2D> s_temp_noise;
static std::unique_ptr<NoiseCache_2D> s_humd_noise;
static std::unique_ptr<NoiseCache_2D> s_cont_noise;
static std::unique_ptr<NoiseCache_2D> s_weird_noise;
static std::unique_ptr<NoiseCache_3D> s_terrain_noise;

static float normalize_01(float sample)
{
    return std::clamp((sample + 1.0f) * 0.5f, 0.0f, 1.0f);
}

static ClimateSample sample_climate(const BlockPosXZ& pos)
{
    ClimateSample sample {};
    sample.temperature = s_temp_noise->sample(pos);
    sample.humidity = s_humd_noise->sample(pos);
    sample.continentalness = s_cont_noise->sample(pos);
    sample.weirdness = s_weird_noise->sample(pos);
    return sample;
}

static bool is_inside_terrain(const BlockPos& bpos, float base_variation, float base_height)
{
    auto variation_noise = s_terrain_noise->sample(bpos);
    auto variation = base_variation * (1.0f - 0.6f * variation_noise * variation_noise);
    auto noise = variation * variation_noise - (static_cast<float>(bpos.y()) - base_height);
    return noise > 0.0f;
}

void realm_surface::init(void)
{
    fnl_state noise_temp = fnlCreateState();
    noise_temp.noise_type = FNL_NOISE_OPENSIMPLEX2;
    noise_temp.frequency = 0.0007f;
    noise_temp.seed = 1;

    fnl_state noise_humd = fnlCreateState();
    noise_humd.noise_type = FNL_NOISE_OPENSIMPLEX2;
    noise_humd.frequency = 0.0007f;
    noise_humd.seed = 2;

    fnl_state noise_cont = fnlCreateState();
    noise_cont.noise_type = FNL_NOISE_OPENSIMPLEX2;
    noise_cont.frequency = 0.00035f;
    noise_cont.seed = 4;

    fnl_state noise_weird = fnlCreateState();
    noise_weird.noise_type = FNL_NOISE_OPENSIMPLEX2;
    noise_weird.frequency = 0.001f;
    noise_weird.seed = 3;

    s_temp_noise = std::make_unique<NoiseCache_2D>(std::move(noise_temp), Eigen::Vector2i(8, 8));
    s_humd_noise = std::make_unique<NoiseCache_2D>(std::move(noise_humd), Eigen::Vector2i(8, 8));
    s_cont_noise = std::make_unique<NoiseCache_2D>(std::move(noise_cont), Eigen::Vector2i(8, 8));
    s_weird_noise = std::make_unique<NoiseCache_2D>(std::move(noise_weird), Eigen::Vector2i(8, 8));

    fnl_state noise_terrain = fnlCreateState();
    noise_terrain.seed = 1337;
    noise_terrain.noise_type = FNL_NOISE_PERLIN;
    noise_terrain.fractal_type = FNL_FRACTAL_FBM;
    noise_terrain.frequency = 0.008f;
    noise_terrain.octaves = 5;
    noise_terrain.gain = 0.55f;

    s_terrain_noise = std::make_unique<NoiseCache_3D>(std::move(noise_terrain), Eigen::Vector3i(4, 8, 4));
}

void realm_surface::shutdown(void)
{
    s_temp_noise.reset();
    s_humd_noise.reset();
    s_cont_noise.reset();
    s_weird_noise.reset();
    s_terrain_noise.reset();
}

bool realm_surface::generate(BlockStorage& storage, const ChunkPos& pos)
{
    thread_local std::array<block_id_type, constant::CHUNK_VOLUME> basic_slice;
    thread_local std::array<block_id_type, constant::CHUNK_VOLUME> filler_slice;
    thread_local std::array<block_id_type, constant::CHUNK_VOLUME> surface_slice;
    thread_local std::array<block_id_type, constant::CHUNK_VOLUME> fluid_slice;
    thread_local std::array<float, constant::CHUNK_VOLUME> variation_slice;
    thread_local std::array<float, constant::CHUNK_VOLUME> base_slice;

    for(std::int16_t lz = 0; lz < constant::CHUNK_SIZE; lz += 1) {
        for(std::int16_t lx = 0; lx < constant::CHUNK_SIZE; lx += 1) {
            auto index = static_cast<std::size_t>(lx + lz * constant::CHUNK_SIZE);
            auto bpos = utils::to_block(pos, LocalPos(lx, 0, lz));
            auto bpos_xz = BlockPosXZ(bpos.x(), bpos.z());

            auto sample = sample_climate(bpos_xz);
            auto biome = climate::find(BIOME_REALM_SURFACE, sample);

            if(biome == nullptr) {
                basic_slice[index] = BLOCK_ID_NULL;
                filler_slice[index] = BLOCK_ID_NULL;
                surface_slice[index] = BLOCK_ID_NULL;
                fluid_slice[index] = BLOCK_ID_NULL;
            }
            else {
                basic_slice[index] = biome->palette_basic.cached;
                filler_slice[index] = biome->palette_filler.cached;
                surface_slice[index] = biome->palette_surface.cached;
                fluid_slice[index] = biome->palette_fluid.cached;
            }

            auto weirdness = normalize_01(sample.weirdness);
            auto continentalness = normalize_01(sample.continentalness);
            variation_slice[index] = std::lerp(VARIATION_MIN, VARIATION_MAX, weirdness);
            base_slice[index] = std::lerp(BASE_MIN, BASE_MAX, continentalness);
        }
    }

    // Pass 1: density basic terrain fill
    for(std::size_t i = 0; i < constant::CHUNK_VOLUME; ++i) {
        auto lpos = utils::to_local(i);
        auto bpos = utils::to_block(pos, lpos);
        auto index = static_cast<std::size_t>(lpos.x() + lpos.z() * constant::CHUNK_SIZE);

        auto variation = variation_slice[index];
        auto base = base_slice[index];
        auto y_rel = static_cast<float>(bpos.y()) - base;

        if(y_rel > variation) {
            continue;
        }

        if(y_rel < -variation) {
            storage.set(i, basic_slice[index]);
            continue;
        }

        if(is_inside_terrain(bpos, variation, base)) {
            storage.set(i, basic_slice[index]);
        }
        else if(bpos.y() < SEA_LEVEL) {
            storage.set(i, fluid_slice[index]);
        }
    }

    // Pass 2: surface skin
    for(std::size_t i = 0; i < constant::CHUNK_VOLUME; ++i) {
        auto lpos = utils::to_local(i);
        auto bpos = utils::to_block(pos, lpos);
        auto index = static_cast<std::size_t>(lpos.x() + lpos.z() * constant::CHUNK_SIZE);

        auto variation = variation_slice[index];
        auto base = base_slice[index];
        auto y_rel = static_cast<float>(bpos.y()) - base;

        if(std::abs(y_rel) > variation) {
            continue;
        }

        auto current = storage.get(i);

        if(current == BLOCK_ID_NULL || current == fluid_slice[index]) {
            continue;
        }

        auto depth = 0U;
        auto underwater = false;

        for(unsigned dy = 0; dy < 5; dy += 1) {
            auto d_lpos = LocalPos(lpos.x(), lpos.y() + static_cast<LocalPos::value_type>(dy + 1), lpos.z());
            auto d_bpos = utils::to_block(pos, d_lpos);
            auto d_index = utils::to_index(d_lpos);

            if(d_lpos.y() >= static_cast<LocalPos::value_type>(constant::CHUNK_SIZE)) {
                if(!is_inside_terrain(d_bpos, variation, base)) {
                    underwater = dy == 0 && d_bpos.y() < SEA_LEVEL;
                    break;
                }
                depth += 1U;
            }
            else {
                auto above = storage.get(d_index);

                if(above == BLOCK_ID_NULL || above == fluid_slice[index]) {
                    underwater = dy == 0 && above == fluid_slice[index];
                    break;
                }
                depth += 1U;
            }
        }

        if(depth < 5) {
            if(underwater || depth > 0) {
                storage.set(i, filler_slice[index]);
            }
            else {
                storage.set(i, surface_slice[index]);
            }
        }
    }

    return true;
}
