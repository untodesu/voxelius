#include "shared/pch.hh"

#include "shared/entity/velocity.hh"

#include "shared/entity/stasis.hh"
#include "shared/entity/transform.hh"
#include "shared/globals.hh"
#include "shared/world/world.hh"

void Velocity::fixed_update(void)
{
    auto view = world::basic_entities.view<Velocity, Transform>(entt::exclude<Stasis>);

    for(auto [entity, velocity, transform] : view.each()) {
        transform.local += velocity.value * globals::fixed_frametime;
    }
}
