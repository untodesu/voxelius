#include "shared/pch.hh"

#include "shared/entity/transform.hh"

#include "shared/constant.hh"
#include "shared/world/world.hh"

constexpr inline static void update_component(unsigned dim, Transform& component)
{
    if(component.local[dim] >= constant::CHUNK_SIZE) {
        component.local[dim] -= constant::CHUNK_SIZE;
        component.chunk[dim] += 1;
        return;
    }

    if(component.local[dim] < 0.0f) {
        component.local[dim] += constant::CHUNK_SIZE;
        component.chunk[dim] -= 1;
        return;
    }
}

void Transform::fixed_update(void)
{
    auto view = world::basic_entities.view<Transform>();

    for(auto [entity, transform] : view.each()) {
        update_component(0U, transform);
        update_component(1U, transform);
        update_component(2U, transform);
    }
}
