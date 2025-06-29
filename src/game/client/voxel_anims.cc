#include "client/pch.hh"

#include "client/voxel_anims.hh"

#include "core/config.hh"
#include "core/constexpr.hh"

#include "client/globals.hh"

static ConfigUnsigned base_framerate(16U, 1U, 16U);

std::uint64_t voxel_anims::nextframe = 0U;
std::uint32_t voxel_anims::frame = 0U;

void voxel_anims::init(void)
{
    globals::client_config.add_value("voxel_anims.base_framerate", base_framerate);

    voxel_anims::nextframe = 0U;
    voxel_anims::frame = 0U;
}

void voxel_anims::update(void)
{
    if(globals::curtime >= voxel_anims::nextframe) {
        voxel_anims::nextframe = globals::curtime + static_cast<std::uint64_t>(1000000.0 / static_cast<float>(base_framerate.get_value()));
        voxel_anims::frame += 1U;
    }
}
