#include "shared/pch.hh"

#include "shared/utils/entity.hh"

#include "shared/head.hh"
#include "shared/player.hh"
#include "shared/transform.hh"
#include "shared/utils/coord.hh"
#include "shared/velocity.hh"
#include "shared/world.hh"

entt::entity utils::spawn_player(const block_pos& pos)
{
    auto entity = world::basic_entities.create();

    if(world::basic_entities.valid(entity)) {
        auto& transform = world::basic_entities.emplace<Transform>(entity);
        transform.angles = Eigen::Vector3f::Zero();
        transform.chunk = utils::to_chunk(pos);
        transform.local = utils::to_local(pos).cast<float>();

        auto& head = world::basic_entities.emplace<Head>(entity);
        head.angles = Eigen::Vector3f::Zero();
        head.offset = Eigen::Vector3f(0.0f, 1.6f, 0.0f);

        auto& velocity = world::basic_entities.emplace<Velocity>(entity);
        velocity.value = Eigen::Vector3f::Zero();

        world::basic_entities.emplace<Player>(entity);
    }

    return entity;
}
