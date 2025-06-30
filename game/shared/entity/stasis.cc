#include "shared/pch.hh"

#include "shared/entity/stasis.hh"

#include "shared/entity/transform.hh"
#include "shared/world/dimension.hh"

void entity::Stasis::fixed_update(world::Dimension* dimension)
{
    auto view = dimension->entities.view<entity::Transform>();

    for(auto [entity, transform] : view.each()) {
        if(dimension->find_chunk(transform.chunk)) {
            dimension->entities.remove<entity::Stasis>(entity);
        }
        else {
            dimension->entities.emplace_or_replace<entity::Stasis>(entity);
        }
    }
}
