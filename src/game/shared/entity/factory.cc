#include "shared/pch.hh"

#include "shared/entity/factory.hh"

#include "shared/entity/collision.hh"
#include "shared/entity/gravity.hh"
#include "shared/entity/head.hh"
#include "shared/entity/player.hh"
#include "shared/entity/transform.hh"
#include "shared/entity/velocity.hh"

#include "shared/world/dimension.hh"

#include "shared/globals.hh"

void entity::shared::create_player(world::Dimension* dimension, entt::entity entity)
{
    spdlog::debug("factory[{}]: assigning player components to {}", dimension->get_name(), static_cast<std::uint64_t>(entity));

    auto& collision = dimension->entities.emplace_or_replace<entity::Collision>(entity);
    collision.aabb.min = glm::fvec3(-0.4f, -1.6f, -0.4f);
    collision.aabb.max = glm::fvec3(+0.4f, +0.2f, +0.4f);

    auto& head = dimension->entities.emplace_or_replace<entity::Head>(entity);
    head.angles = glm::fvec3(0.0f, 0.0f, 0.0f);
    head.offset = glm::fvec3(0.0f, 0.0f, 0.0f);

    dimension->entities.emplace_or_replace<entity::Player>(entity);

    auto& transform = dimension->entities.emplace_or_replace<entity::Transform>(entity);
    transform.chunk = chunk_pos(0, 2, 0);
    transform.local = glm::fvec3(0.0f, 0.0f, 0.0f);
    transform.angles = glm::fvec3(0.0f, 0.0f, 0.0f);

    auto& velocity = dimension->entities.emplace_or_replace<entity::Velocity>(entity);
    velocity.value = glm::fvec3(0.0f, 0.0f, 0.0f);
}
