#include "shared/pch.hh"

#include "shared/entity/gravity.hh"

#include "shared/entity/stasis.hh"
#include "shared/entity/velocity.hh"

#include "shared/world/dimension.hh"

#include "shared/globals.hh"

void entity::Gravity::fixed_update(world::Dimension* dimension)
{
    auto fixed_acceleration = globals::fixed_frametime * dimension->get_gravity();
    auto group = dimension->entities.group<entity::Gravity>(entt::get<entity::Velocity>, entt::exclude<entity::Stasis>);

    for(auto [entity, velocity] : group.each()) {
        velocity.value.y += fixed_acceleration;
    }
}
