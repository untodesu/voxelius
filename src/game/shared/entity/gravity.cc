#include "shared/pch.hh"

#include "shared/entity/gravity.hh"

#include "shared/entity/stasis.hh"
#include "shared/entity/velocity.hh"

#include "shared/world/dimension.hh"

#include "shared/globals.hh"

void Gravity::fixed_update(Dimension* dimension)
{
    auto fixed_acceleration = globals::fixed_frametime * dimension->get_gravity();
    auto group = dimension->entities.group<Gravity>(entt::get<Velocity>, entt::exclude<Stasis>);

    for(auto [entity, velocity] : group.each()) {
        velocity.value.y += fixed_acceleration;
    }
}
