#include "shared/pch.hh"

#include "shared/velocity.hh"

#include "shared/globals.hh"
#include "shared/stasis.hh"
#include "shared/transform.hh"
#include "shared/world.hh"

void Velocity::fixed_update(void)
{
    auto view = world::basic_entities.view<Velocity, Transform>(entt::exclude<Stasis>);

    for(auto [entity, velocity, transform] : view.each()) {
        transform.local += velocity.value * globals::fixed_frametime;

        LOG_INFO("{} {} {} // {} {} {}", velocity.value.x(), velocity.value.y(), velocity.value.z(), transform.local.x(),
            transform.local.y(), transform.local.z());
    }
}
