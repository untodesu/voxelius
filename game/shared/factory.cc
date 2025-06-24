#include "shared/pch.hh"

#include "shared/factory.hh"

#include "shared/collision.hh"
#include "shared/dimension.hh"
#include "shared/globals.hh"
#include "shared/gravity.hh"
#include "shared/head.hh"
#include "shared/player.hh"
#include "shared/transform.hh"
#include "shared/velocity.hh"

void shared_factory::create_player(Dimension* dimension, entt::entity entity)
{
    spdlog::debug("factory[{}]: assigning player components to {}", dimension->get_name(), static_cast<std::uint64_t>(entity));

    auto& collision = dimension->entities.emplace_or_replace<CollisionComponent>(entity);
    collision.aabb.min = glm::fvec3(-0.4f, -1.6f, -0.4f);
    collision.aabb.max = glm::fvec3(+0.4f, +0.2f, +0.4f);

    auto& head = dimension->entities.emplace_or_replace<HeadComponent>(entity);
    head.angles = glm::fvec3(0.0f, 0.0f, 0.0f);
    head.offset = glm::fvec3(0.0f, 0.0f, 0.0f);

    dimension->entities.emplace_or_replace<PlayerComponent>(entity);

    auto& transform = dimension->entities.emplace_or_replace<TransformComponent>(entity);
    transform.chunk = chunk_pos(0, 2, 0);
    transform.local = glm::fvec3(0.0f, 0.0f, 0.0f);
    transform.angles = glm::fvec3(0.0f, 0.0f, 0.0f);

    auto& velocity = dimension->entities.emplace_or_replace<VelocityComponent>(entity);
    velocity.value = glm::fvec3(0.0f, 0.0f, 0.0f);
}
