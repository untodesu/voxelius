#include "client/pch.hh"

#include "client/entity/factory.hh"

#include "shared/entity/factory.hh"
#include "shared/entity/head.hh"
#include "shared/entity/transform.hh"
#include "shared/entity/velocity.hh"

#include "shared/world/dimension.hh"

#include "client/entity/sound_emitter.hh"

#include "client/globals.hh"

void client::create_player(Dimension* dimension, entt::entity entity)
{
    shared::create_player(dimension, entity);

    const auto& head = dimension->entities.get<Head>(entity);
    dimension->entities.emplace_or_replace<client::HeadIntr>(entity, head);
    dimension->entities.emplace_or_replace<client::HeadPrev>(entity, head);

    const auto& transform = dimension->entities.get<Transform>(entity);
    dimension->entities.emplace_or_replace<client::TransformIntr>(entity, transform);
    dimension->entities.emplace_or_replace<client::TransformPrev>(entity, transform);

    const auto& velocity = dimension->entities.get<Velocity>(entity);
    dimension->entities.emplace_or_replace<client::VelocityIntr>(entity, velocity);
    dimension->entities.emplace_or_replace<client::VelocityPrev>(entity, velocity);

    if(globals::sound_ctx) {
        dimension->entities.emplace_or_replace<SoundEmitter>(entity);
    }
}
