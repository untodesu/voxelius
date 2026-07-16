#include "client/pch.hh"

#include "client/game.hh"

#include "core/camera.hh"
#include "core/identifier.hh"
#include "core/utils/angles.hh"

#include "shared/constant.hh"
#include "shared/coord.hh"
#include "shared/entity/transform.hh"
#include "shared/entity/velocity.hh"
#include "shared/physics/physics.hh"
#include "shared/utils/coord.hh"
#include "shared/utils/world.hh"
#include "shared/world/biome_lut.hh"
#include "shared/world/block_registry.hh"
#include "shared/world/world.hh"

#include "client/entity/camera.hh"
#include "client/entity/interpolation.hh"
#include "client/entity/player_look.hh"
#include "client/entity/player_move.hh"
#include "client/entity/player_target.hh"
#include "client/globals.hh"
#include "client/gui/gui.hh"
#include "client/utils/entity.hh"

#include <fastnoiselite.h>

static void generate_debug_terrain(void)
{
    auto stone_id = block_registry::find(Identifier::from_string("builtin:stone"));
    auto slab_id = block_registry::find(Identifier::from_string("builtin:stone_slab"));
    auto dirt_id = block_registry::find(Identifier::from_string("builtin:dirt"));
    auto grass_id = block_registry::find(Identifier::from_string("builtin:grass"));

    if(stone_id == BLOCK_ID_NULL || slab_id == BLOCK_ID_NULL || dirt_id == BLOCK_ID_NULL || grass_id == BLOCK_ID_NULL) {
        LOG_WARNING("builtin:stone/stone_slab/dirt/grass not found, skipping debug terrain");
        return;
    }

    auto bush_id = block_registry::find(Identifier::from_string("builtin:bush"));

    block_id_type slab_bottom_id = slab_id;

    if(auto* family = block_registry::find_family_of(slab_id)) {
        const auto orientation_key = family->state_hash("orientation");

        emhash8::HashMap<blockstate_key_type, blockstate_val_type> bottom_map;
        bottom_map.try_emplace(orientation_key, family->state_hash("bottom"));
        slab_bottom_id = block_registry::resolve_variant(slab_id, bottom_map);
    }

    fnl_state noise = fnlCreateState();
    noise.noise_type = FNL_NOISE_OPENSIMPLEX2;
    noise.fractal_type = FNL_FRACTAL_FBM;
    noise.frequency = 0.02f;
    noise.octaves = 4;
    noise.lacunarity = 2.0f;
    noise.gain = 0.5f;

    // Three independent low-frequency noise fields drive the biome LUT lookup
    // (temperature/humidity/a third climate axis), separate from the high
    // frequency `noise` above that only shapes the terrain height
    fnl_state noise_temp = fnlCreateState();
    noise_temp.noise_type = FNL_NOISE_OPENSIMPLEX2;
    noise_temp.frequency = 0.003f;
    noise_temp.seed = 1;

    // make temperature noise ridged
    noise_temp.fractal_type = FNL_FRACTAL_RIDGED;
    noise_temp.weighted_strength = 0.5f;

    fnl_state noise_humd = fnlCreateState();
    noise_humd.noise_type = FNL_NOISE_OPENSIMPLEX2;
    noise_humd.frequency = 0.003f;
    noise_humd.seed = 2;

    fnl_state noise_axis = fnlCreateState();
    noise_axis.noise_type = FNL_NOISE_OPENSIMPLEX2;
    noise_axis.frequency = 0.003f;
    noise_axis.seed = 3;

    auto to_lut_coord = [](float sample) -> std::uint8_t {
        const float normalized = std::clamp((sample + 1.0f) * 0.5f, 0.0f, 1.0f);
        return static_cast<std::uint8_t>(normalized * 99.0f);
    };

    constexpr std::int32_t CHUNK_RADIUS = 32;
    constexpr std::int32_t SIZE = static_cast<std::int32_t>(constant::CHUNK_SIZE);
    constexpr std::int32_t BASE_HEIGHT = 8;
    constexpr std::int32_t AMPLITUDE = 5;
    constexpr std::int32_t DIRT_DEPTH = 4;

    std::mt19937_64 rng(std::random_device {}());

    for(std::int32_t cx = -CHUNK_RADIUS; cx <= CHUNK_RADIUS; cx += 1) {
        for(std::int32_t cz = -CHUNK_RADIUS; cz <= CHUNK_RADIUS; cz += 1) {
            const ChunkPos cpos(cx, 0, cz);
            auto chunk = world::create_chunk(cpos);

            for(std::int32_t x = 0; x < SIZE; x += 1) {
                for(std::int32_t z = 0; z < SIZE; z += 1) {
                    const float wx = static_cast<float>(cx * SIZE + x);
                    const float wz = static_cast<float>(cz * SIZE + z);
                    const float sample = fnlGetNoise2D(&noise, wx, wz);

                    const std::uint8_t lut_temp = to_lut_coord(fnlGetNoise2D(&noise_temp, wx, wz));
                    const std::uint8_t lut_humd = to_lut_coord(fnlGetNoise2D(&noise_humd, wx, wz));
                    const std::uint8_t lut_axis = to_lut_coord(fnlGetNoise2D(&noise_axis, wx, wz));

                    const auto* biome = biome_lut::find(BIOME_REALM_SURFACE, lut_temp, lut_humd, lut_axis);

                    if(biome == nullptr) {
                        LOG_INFO("{}", static_cast<const void*>(biome));
                    }

                    block_id_type col_basic_id = stone_id;
                    block_id_type col_filler_id = dirt_id;
                    block_id_type col_surface_id = grass_id;

                    if(biome != nullptr) {
                        col_basic_id = biome->palette_basic.cached;
                        col_filler_id = biome->palette_filler.cached;
                        col_surface_id = biome->palette_surface.cached;
                    }

                    const std::int32_t height = BASE_HEIGHT + static_cast<std::int32_t>(sample * static_cast<float>(AMPLITUDE));
                    const std::int32_t top = std::clamp(height, 1, SIZE - 2);
                    const std::int32_t dirt_start = std::max(top - DIRT_DEPTH, 0);

                    for(std::int32_t y = 0; y < dirt_start; y += 1)
                        chunk->set_block(LocalPos(x, y, z), col_basic_id);

                    for(std::int32_t y = dirt_start; y < top; y += 1)
                        chunk->set_block(LocalPos(x, y, z), col_filler_id);

                    if((x * SIZE + z) % 37 == 0) {
                        chunk->set_block(LocalPos(x, top, z), slab_bottom_id);
                    }
                    else {
                        chunk->set_block(LocalPos(x, top, z), col_surface_id);
                    }

                    auto chance_1 = std::uniform_int_distribution<std::int32_t>(0, 100)(rng);
                    auto chance_2 = std::uniform_int_distribution<std::int32_t>(0, 100)(rng);

                    if(bush_id && col_surface_id == grass_id && top + 1 < SIZE && chance_1 < 50) {
                        chunk->set_block(LocalPos(x, top + 1, z), bush_id);
                    }
                }
            }
        }
    }
}

void client_game::init(void)
{
    player_look::init();
    player_move::init();
    player_target::init();
}

void client_game::init_late(void)
{
    globals::player = utils::spawn_player_client({ -8, 24, -8 });

    generate_debug_terrain();
}

void client_game::shutdown(void)
{
    // empty
}

void client_game::update(void)
{
    interpolation::update();

    player_target::update();
}

void client_game::update_late(void)
{
    player_look::update_late();
    player_move::update_late();
}

void client_game::fixed_update(void)
{
    player_move::fixed_update();

    Transform::fixed_update();
    Velocity::fixed_update();

    world::fixed_update();
}

void client_game::fixed_update_late(void)
{
    // empty
}

void client_game::layout(void)
{
    // empty
}

void client_game::render(void)
{
    player_target::render();
}
