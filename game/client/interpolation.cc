#include "client/pch.hh"
#include "client/interpolation.hh"

#include "core/constexpr.hh"

#include "shared/coord.hh"
#include "shared/dimension.hh"
#include "shared/head.hh"
#include "shared/transform.hh"

#include "client/globals.hh"

static void transform_interpolate(float alpha)
{
    auto group = globals::dimension->entities.group<TransformComponentIntr>(entt::get<TransformComponent, TransformComponentPrev>);

    for(auto [entity, interp, current, previous] : group.each()) {
        interp.angles[0] = cxpr::lerp(previous.angles[0], current.angles[0], alpha);
        interp.angles[1] = cxpr::lerp(previous.angles[1], current.angles[1], alpha);
        interp.angles[2] = cxpr::lerp(previous.angles[2], current.angles[2], alpha);

        // Figure out previous chunk-local floating-point coordinates transformed
        // to the current WorldCoord's chunk domain coordinates; we're interpolating
        // against these instead of using previous.position.local to prevent jittering
        auto previous_shift = coord::to_relative(current.chunk, current.local, previous.chunk, previous.local);
        auto previous_local = current.local + previous_shift;

        interp.chunk.x = current.chunk.x;
        interp.chunk.y = current.chunk.y;
        interp.chunk.z = current.chunk.z;

        interp.local.x = cxpr::lerp(previous_local.x, current.local.x, alpha);
        interp.local.y = cxpr::lerp(previous_local.y, current.local.y, alpha);
        interp.local.z = cxpr::lerp(previous_local.z, current.local.z, alpha);
    }
}

static void head_interpolate(float alpha)
{
    auto group = globals::dimension->entities.group<HeadComponentIntr>(entt::get<HeadComponent, HeadComponentPrev>);

    for(auto [entity, interp, current, previous] : group.each()) {
        interp.angles[0] = cxpr::lerp(previous.angles[0], current.angles[0], alpha);
        interp.angles[1] = cxpr::lerp(previous.angles[1], current.angles[1], alpha);
        interp.angles[2] = cxpr::lerp(previous.angles[2], current.angles[2], alpha);

        interp.offset.x = cxpr::lerp(previous.offset.x, current.offset.x, alpha);
        interp.offset.y = cxpr::lerp(previous.offset.y, current.offset.y, alpha);
        interp.offset.z = cxpr::lerp(previous.offset.z, current.offset.z, alpha);
    }
}

void interpolation::update(void)
{
    if(globals::dimension) {
        auto alpha = static_cast<float>(globals::fixed_accumulator) / static_cast<float>(globals::fixed_frametime_us);
        transform_interpolate(alpha);
        head_interpolate(alpha);
    }
}