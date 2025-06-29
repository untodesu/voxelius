#include "shared/pch.hh"

#include "shared/velocity.hh"

#include "shared/dimension.hh"
#include "shared/globals.hh"
#include "shared/stasis.hh"
#include "shared/transform.hh"

void VelocityComponent::fixed_update(Dimension* dimension)
{
    auto group = dimension->entities.group<VelocityComponent>(entt::get<TransformComponent>, entt::exclude<StasisComponent>);

    for(auto [entity, velocity, transform] : group.each()) {
        transform.local += velocity.value * globals::fixed_frametime;
    }
}
