#include "shared/pch.hh"

#include "shared/component/stasis.hh"

#include "shared/component/transform.hh"
#include "shared/world/world.hh"

void Stasis_Component::fixed_update(void)
{
    auto view = world::basic_entities.view<Transform>();

    for(auto [entity, transform] : view.each()) {
        if(world::find_chunk(transform.chunk)) {
            world::basic_entities.remove<Stasis_Component>(entity);
        }
        else {
            world::basic_entities.emplace_or_replace<Stasis_Component>(entity);
        }
    }
}
