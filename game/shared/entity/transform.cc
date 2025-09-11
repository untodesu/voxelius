#include "shared/pch.hh"

#include "shared/entity/transform.hh"

#include "shared/world/dimension.hh"

#include "shared/const.hh"

constexpr inline static void update_component(unsigned int dim, entity::Transform& component)
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

void entity::Transform::fixed_update(world::Dimension* dimension)
{
    auto view = dimension->entities.view<entity::Transform>();

    for(auto [entity, transform] : view.each()) {
        update_component(0U, transform);
        update_component(1U, transform);
        update_component(2U, transform);
    }
}
