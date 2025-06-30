#include "shared/pch.hh"

#include "shared/entity/velocity.hh"

#include "shared/entity/stasis.hh"
#include "shared/entity/transform.hh"
#include "shared/world/dimension.hh"

#include "shared/globals.hh"

void entity::Velocity::fixed_update(world::Dimension* dimension)
{
    auto group = dimension->entities.group<entity::Velocity>(entt::get<entity::Transform>, entt::exclude<entity::Stasis>);

    for(auto [entity, velocity, transform] : group.each()) {
        transform.local += velocity.value * globals::fixed_frametime;
    }
}
