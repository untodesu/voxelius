#include "client/pch.hh"

#include "client/world/voxel_anims.hh"

#include "core/config/number.hh"

#include "core/io/config_map.hh"

#include "core/math/constexpr.hh"

#include "client/globals.hh"

static config::Unsigned base_framerate(16U, 1U, 16U);

std::uint64_t world::voxel_anims::nextframe = 0U;
std::uint32_t world::voxel_anims::frame = 0U;

void world::voxel_anims::init(void)
{
    globals::client_config.add_value("voxel_anims.base_framerate", base_framerate);

    world::voxel_anims::nextframe = 0U;
    world::voxel_anims::frame = 0U;
}

void world::voxel_anims::update(void)
{
    if(globals::curtime >= world::voxel_anims::nextframe) {
        world::voxel_anims::nextframe = globals::curtime
                + static_cast<std::uint64_t>(1000000.0 / static_cast<float>(base_framerate.get_value()));
        world::voxel_anims::frame += 1U;
    }
}
