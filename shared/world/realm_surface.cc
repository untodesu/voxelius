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
constexpr static float VARIATION_MAX = 128.0f;
constexpr static float BASE_MIN = -24.0f;
constexpr static float BASE_MAX = 16.0f;

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
    const auto variation_noise = s_terrain_noise->sample(bpos);
    const auto variation = base_variation * (1.0f - (variation_noise * variation_noise));
    const auto noise = variation * variation_noise - (static_cast<float>(bpos.y()) - base_height);
    return noise > 0.0f;
}

void realm_surface::init(void)
{
    // Low-frequency climate: large coherent regions (not blotches).
    // Avoid ridged temperature — it shreds Voronoi cells into speckles.
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

    // Coarser cache cells — climate varies slowly
    s_temp_noise = std::make_unique<NoiseCache_2D>(std::move(noise_temp), Eigen::Vector2i(8, 8));
    s_humd_noise = std::make_unique<NoiseCache_2D>(std::move(noise_humd), Eigen::Vector2i(8, 8));
    s_cont_noise = std::make_unique<NoiseCache_2D>(std::move(noise_cont), Eigen::Vector2i(8, 8));
    s_weird_noise = std::make_unique<NoiseCache_2D>(std::move(noise_weird), Eigen::Vector2i(8, 8));

    fnl_state noise_terrain = fnlCreateState();
    noise_terrain.seed = 1337;
    noise_terrain.noise_type = FNL_NOISE_OPENSIMPLEX2S;
    noise_terrain.fractal_type = FNL_FRACTAL_FBM;
    noise_terrain.frequency = 0.005f;
    noise_terrain.octaves = 4;

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
    auto fallback_basic = block_registry::find(Identifier::from_string("builtin:stone"));
    auto fallback_filler = block_registry::find(Identifier::from_string("builtin:dirt"));
    auto fallback_surface = block_registry::find(Identifier::from_string("builtin:grass"));

    std::array<block_id_type, constant::CHUNK_AREA> col_basic;
    std::array<block_id_type, constant::CHUNK_AREA> col_filler;
    std::array<block_id_type, constant::CHUNK_AREA> col_surface;
    std::array<float, constant::CHUNK_AREA> col_variation;
    std::array<float, constant::CHUNK_AREA> col_base;

    for(std::int32_t lz = 0; lz < static_cast<std::int32_t>(constant::CHUNK_SIZE); ++lz) {
        for(std::int32_t lx = 0; lx < static_cast<std::int32_t>(constant::CHUNK_SIZE); ++lx) {
            auto hdx = static_cast<std::size_t>(lx + lz * static_cast<std::int32_t>(constant::CHUNK_SIZE));
            auto bpos = utils::to_block(pos, LocalPos(lx, 0, lz));
            auto xz = BlockPosXZ(bpos.x(), bpos.z());
            auto climate_sample = sample_climate(xz);
            auto biome = climate::find(BIOME_REALM_SURFACE, climate_sample);

            col_basic[hdx] = (biome != nullptr) ? biome->palette_basic.cached : fallback_basic;
            col_filler[hdx] = (biome != nullptr) ? biome->palette_filler.cached : fallback_filler;
            col_surface[hdx] = (biome != nullptr) ? biome->palette_surface.cached : fallback_surface;

            // Shape from continuous climate only — avoids cliffs at biome borders.
            // Desert stays flat because it nucleates at low weirdness (weighted in climate::find).
            const float weird_01 = normalize_01(climate_sample.weirdness);
            const float cont_01 = normalize_01(climate_sample.continentalness);
            col_variation[hdx] = std::lerp(VARIATION_MIN, VARIATION_MAX, weird_01);
            col_base[hdx] = std::lerp(BASE_MIN, BASE_MAX, cont_01);
        }
    }

    // Pass 1: density terrain fill (basic / stone)
    for(std::size_t i = 0; i < constant::CHUNK_VOLUME; ++i) {
        auto lpos = utils::to_local(i);
        auto bpos = utils::to_block(pos, lpos);
        auto hdx = static_cast<std::size_t>(lpos.x() + lpos.z() * static_cast<LocalPos::value_type>(constant::CHUNK_SIZE));
        auto variation = col_variation[hdx];
        auto base = col_base[hdx];
        auto y_rel = static_cast<float>(bpos.y()) - base;

        if(y_rel > variation) {
            continue;
        }

        if(y_rel < -variation) {
            storage.set(i, col_basic[hdx]);
            continue;
        }

        if(is_inside_terrain(bpos, variation, base)) {
            storage.set(i, col_basic[hdx]);
        }
    }

    // Pass 2: surface skin (surface / filler)
    for(std::size_t i = 0; i < constant::CHUNK_VOLUME; ++i) {
        auto lpos = utils::to_local(i);
        auto bpos = utils::to_block(pos, lpos);
        auto hdx = static_cast<std::size_t>(lpos.x() + lpos.z() * static_cast<LocalPos::value_type>(constant::CHUNK_SIZE));
        auto variation = col_variation[hdx];
        auto base = col_base[hdx];
        auto y_rel = static_cast<float>(bpos.y()) - base;

        if((y_rel > variation) || (y_rel < -variation)) {
            continue;
        }

        if(storage.get(i) == BLOCK_ID_NULL) {
            continue;
        }

        unsigned int depth = 0U;

        for(unsigned int dy = 0U; dy < 5U; dy += 1U) {
            auto d_lpos = LocalPos(lpos.x(), lpos.y() + static_cast<LocalPos::value_type>(dy + 1), lpos.z());
            auto d_bpos = utils::to_block(pos, d_lpos);

            if(d_lpos.y() >= static_cast<LocalPos::value_type>(constant::CHUNK_SIZE)) {
                if(!is_inside_terrain(d_bpos, variation, base)) {
                    break;
                }

                depth += 1U;
            }
            else {
                auto d_index = utils::to_index(d_lpos);

                if(storage.get(d_index) == BLOCK_ID_NULL) {
                    break;
                }

                depth += 1U;
            }
        }

        if(depth < 5U) {
            if(depth == 0U) {
                storage.set(i, col_surface[hdx]);
            }
            else {
                storage.set(i, col_filler[hdx]);
            }
        }
    }

    return true;
}
