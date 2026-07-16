#include "shared/pch.hh"

#include "shared/entity/stasis.hh"

#include "shared/entity/transform.hh"
#include "shared/world/world.hh"

void Stasis::fixed_update(void)
{
    auto view = world::basic_entities.view<Transform>();

    for(auto [entity, transform] : view.each()) {
        if(world::find_chunk(transform.chunk)) {
            world::basic_entities.remove<Stasis>(entity);
        }
        else {
            world::basic_entities.emplace_or_replace<Stasis>(entity);
        }
    }
}
