#include "shared/pch.hh"

#include "shared/transform.hh"

#include "shared/const.hh"
#include "shared/dimension.hh"

constexpr inline static void update_component(unsigned int dim, TransformComponent& component)
{
    if(component.local[dim] >= CHUNK_SIZE) {
        component.local[dim] -= CHUNK_SIZE;
        component.chunk[dim] += 1;
        return;
    }

    if(component.local[dim] < 0.0f) {
        component.local[dim] += CHUNK_SIZE;
        component.chunk[dim] -= 1;
        return;
    }
}

void TransformComponent::fixed_update(Dimension* dimension)
{
    auto view = dimension->entities.view<TransformComponent>();

    for(auto [entity, transform] : view.each()) {
        update_component(0U, transform);
        update_component(1U, transform);
        update_component(2U, transform);
    }
}
