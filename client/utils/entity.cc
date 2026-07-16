#include "client/pch.hh"

#include "client/utils/entity.hh"

#include "shared/entity/head.hh"
#include "shared/entity/transform.hh"
#include "shared/entity/velocity.hh"
#include "shared/utils/entity.hh"
#include "shared/world/world.hh"

entt::entity utils::spawn_player_client(const BlockPos& pos)
{
    auto entity = utils::spawn_player(pos);

    if(world::basic_entities.valid(entity)) {
        const auto& transform = world::basic_entities.get<Transform>(entity);
        const auto& head = world::basic_entities.get<Head>(entity);
        const auto& velocity = world::basic_entities.get<Velocity>(entity);

        Transform_Intr transform_intr;
        transform_intr.angles = transform.angles;
        transform_intr.chunk = transform.chunk;
        transform_intr.local = transform.local;
        world::basic_entities.emplace<Transform_Intr>(entity, std::move(transform_intr));

        Transform_Prev transform_prev;
        transform_prev.angles = transform.angles;
        transform_prev.chunk = transform.chunk;
        transform_prev.local = transform.local;
        world::basic_entities.emplace<Transform_Prev>(entity, std::move(transform_prev));

        Head_Intr head_intr;
        head_intr.angles = head.angles;
        head_intr.offset = head.offset;
        world::basic_entities.emplace<Head_Intr>(entity, std::move(head_intr));

        Head_Prev head_prev;
        head_prev.angles = head.angles;
        head_prev.offset = head.offset;
        world::basic_entities.emplace<Head_Prev>(entity, std::move(head_prev));

        Velocity_Intr velocity_intr;
        velocity_intr.value = velocity.value;
        world::basic_entities.emplace<Velocity_Intr>(entity, std::move(velocity_intr));

        Velocity_Prev velocity_prev;
        velocity_prev.value = velocity.value;
        world::basic_entities.emplace<Velocity_Prev>(entity, std::move(velocity_prev));
    }

    return entity;
}
