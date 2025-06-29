#include "shared/pch.hh"

#include "shared/gravity.hh"

#include "shared/dimension.hh"
#include "shared/globals.hh"
#include "shared/stasis.hh"
#include "shared/velocity.hh"

void GravityComponent::fixed_update(Dimension* dimension)
{
    auto fixed_acceleration = globals::fixed_frametime * dimension->get_gravity();
    auto group = dimension->entities.group<GravityComponent>(entt::get<VelocityComponent>, entt::exclude<StasisComponent>);

    for(auto [entity, velocity] : group.each()) {
        velocity.value.y += fixed_acceleration;
    }
}
