#include "server/pch.hh"

#include "server/world/random_tick.hh"

#include "core/config/number.hh"

#include "core/io/config_map.hh"

#include "shared/world/chunk.hh"
#include "shared/world/dimension.hh"
#include "shared/world/voxel.hh"

#include "shared/coord.hh"

#include "server/globals.hh"

static config::Int random_tick_speed(2, 1, 1000);
static std::mt19937_64 random_source;

void world::random_tick::init(void)
{
    globals::server_config.add_value("world.random_tick_speed", random_tick_speed);

    random_source.seed(std::random_device {}());
}

void world::random_tick::tick(const chunk_pos& cpos, Chunk* chunk)
{
    assert(chunk);

    for(int i = 0; i < random_tick_speed.get_value(); ++i) {
        auto voxel_index = random_source() % CHUNK_VOLUME;
        auto lpos = coord::to_local(voxel_index);
        auto vpos = coord::to_voxel(cpos, lpos);

        if(auto voxel = chunk->get_voxel(lpos)) {
            voxel->on_tick(chunk->get_dimension(), vpos);
        }
    }
}
