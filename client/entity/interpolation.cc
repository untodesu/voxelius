#include "client/pch.hh"

#include "client/entity/interpolation.hh"

#include "shared/entity/head.hh"
#include "shared/entity/transform.hh"
#include "shared/entity/velocity.hh"
#include "shared/utils/coord.hh"
#include "shared/world/world.hh"

#include "client/globals.hh"

static void transform_interpolate(float alpha)
{
    auto view = world::basic_entities.view<Transform, Transform_Prev, Transform_Intr>();

    for(auto [entity, current, previous, interp] : view.each()) {
        interp.angles[0] = std::lerp(previous.angles[0], current.angles[0], alpha);
        interp.angles[1] = std::lerp(previous.angles[1], current.angles[1], alpha);
        interp.angles[2] = std::lerp(previous.angles[2], current.angles[2], alpha);

        // Figure out previous chunk-local floating-point coordinates transformed
        // to the current WorldCoord's chunk domain coordinates; we're interpolating
        // against these instead of using previous.position.local to prevent jittering
        auto previous_shift = utils::to_relative(current.chunk, current.local, previous.chunk, previous.local);
        auto previous_local = current.local + previous_shift;

        interp.chunk = current.chunk;

        interp.local.x() = std::lerp(previous_local.x(), current.local.x(), alpha);
        interp.local.y() = std::lerp(previous_local.y(), current.local.y(), alpha);
        interp.local.z() = std::lerp(previous_local.z(), current.local.z(), alpha);
    }
}

static void head_interpolate(float alpha)
{
    auto view = world::basic_entities.view<Head, Head_Prev, Head_Intr>();

    for(auto [entity, current, previous, interp] : view.each()) {
        interp.angles[0] = std::lerp(previous.angles[0], current.angles[0], alpha);
        interp.angles[1] = std::lerp(previous.angles[1], current.angles[1], alpha);
        interp.angles[2] = std::lerp(previous.angles[2], current.angles[2], alpha);

        interp.offset.x() = std::lerp(previous.offset.x(), current.offset.x(), alpha);
        interp.offset.y() = std::lerp(previous.offset.y(), current.offset.y(), alpha);
        interp.offset.z() = std::lerp(previous.offset.z(), current.offset.z(), alpha);
    }
}

static void velocity_interpolate(float alpha)
{
    auto view = world::basic_entities.view<Velocity, Velocity_Prev, Velocity_Intr>();

    for(auto [entity, current, previous, interp] : view.each()) {
        interp.value.x() = std::lerp(previous.value.x(), current.value.x(), alpha);
        interp.value.y() = std::lerp(previous.value.y(), current.value.y(), alpha);
        interp.value.z() = std::lerp(previous.value.z(), current.value.z(), alpha);
    }
}

void interpolation::update(void)
{
    auto alpha = static_cast<float>(globals::fixed_accumulator_us) / static_cast<float>(globals::fixed_frametime_us);
    transform_interpolate(alpha);
    head_interpolate(alpha);
    velocity_interpolate(alpha);
}
