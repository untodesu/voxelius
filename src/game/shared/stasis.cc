#include "shared/pch.hh"

#include "shared/stasis.hh"

#include "shared/dimension.hh"
#include "shared/transform.hh"

void StasisComponent::fixed_update(Dimension* dimension)
{
    auto view = dimension->entities.view<TransformComponent>();

    for(auto [entity, transform] : view.each()) {
        if(dimension->find_chunk(transform.chunk)) {
            dimension->entities.remove<StasisComponent>(entity);
        } else {
            dimension->entities.emplace_or_replace<StasisComponent>(entity);
        }
    }
}
