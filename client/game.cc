#include "client/pch.hh"

#include "client/game.hh"

#include "core/camera.hh"
#include "core/identifier.hh"
#include "core/utils/angles.hh"

#include "shared/block_registry.hh"
#include "shared/constant.hh"
#include "shared/coord.hh"
#include "shared/physics.hh"
#include "shared/transform.hh"
#include "shared/utils/coord.hh"
#include "shared/utils/world.hh"
#include "shared/velocity.hh"
#include "shared/world.hh"

#include "client/camera.hh"
#include "client/globals.hh"
#include "client/gui.hh"
#include "client/interpolation.hh"
#include "client/player_look.hh"
#include "client/player_move.hh"
#include "client/player_target.hh"
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

    constexpr std::int32_t CHUNK_RADIUS = 16;
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

                    const std::int32_t height = BASE_HEIGHT + static_cast<std::int32_t>(sample * static_cast<float>(AMPLITUDE));
                    const std::int32_t top = std::clamp(height, 1, SIZE - 2);
                    const std::int32_t dirt_start = std::max(top - DIRT_DEPTH, 0);

                    for(std::int32_t y = 0; y < dirt_start; y += 1)
                        chunk->set_block(LocalPos(x, y, z), stone_id);

                    for(std::int32_t y = dirt_start; y < top; y += 1)
                        chunk->set_block(LocalPos(x, y, z), dirt_id);

                    if((x * SIZE + z) % 37 == 0) {
                        chunk->set_block(LocalPos(x, top, z), slab_bottom_id);
                    }
                    else {
                        chunk->set_block(LocalPos(x, top, z), grass_id);
                    }

                    auto chance_1 = std::uniform_int_distribution<std::int32_t>(0, 100)(rng);
                    auto chance_2 = std::uniform_int_distribution<std::int32_t>(0, 100)(rng);

                    if(bush_id && top + 1 < SIZE && chance_1 < 50) {
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
