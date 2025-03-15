#include "client/pch.hh"
#include "client/factory.hh"

#include "shared/dimension.hh"
#include "shared/factory.hh"
#include "shared/head.hh"
#include "shared/transform.hh"

#include "client/sound_emitter.hh"

void client_factory::create_player(Dimension *dimension, entt::entity entity)
{
    shared_factory::create_player(dimension, entity);

    const auto &head = dimension->entities.get<HeadComponent>(entity);
    dimension->entities.emplace_or_replace<HeadComponentIntr>(entity, head);
    dimension->entities.emplace_or_replace<HeadComponentPrev>(entity, head);

    const auto &transform = dimension->entities.get<TransformComponent>(entity);
    dimension->entities.emplace_or_replace<TransformComponentIntr>(entity, transform);
    dimension->entities.emplace_or_replace<TransformComponentPrev>(entity, transform);

    dimension->entities.emplace_or_replace<SoundEmitterComponent>(entity);
}
