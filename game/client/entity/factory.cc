#include "client/pch.hh"

#include "client/entity/factory.hh"

#include "shared/entity/factory.hh"
#include "shared/entity/head.hh"
#include "shared/entity/transform.hh"
#include "shared/world/dimension.hh"

#include "client/entity/sound_emitter.hh"

#include "client/globals.hh"

void entity::client::create_player(world::Dimension* dimension, entt::entity entity)
{
    entity::shared::create_player(dimension, entity);

    const auto& head = dimension->entities.get<entity::Head>(entity);
    dimension->entities.emplace_or_replace<entity::client::HeadIntr>(entity, head);
    dimension->entities.emplace_or_replace<entity::client::HeadPrev>(entity, head);

    const auto& transform = dimension->entities.get<entity::Transform>(entity);
    dimension->entities.emplace_or_replace<entity::client::TransformIntr>(entity, transform);
    dimension->entities.emplace_or_replace<entity::client::TransformPrev>(entity, transform);

    if(globals::sound_ctx) {
        dimension->entities.emplace_or_replace<entity::SoundEmitter>(entity);
    }
}
