#ifndef SHARED_ENTITY_FACTORY_HH
#define SHARED_ENTITY_FACTORY_HH 1
#pragma once

namespace world
{
class Dimension;
} // namespace world

namespace entity::shared
{
void create_player(world::Dimension* dimension, entt::entity entity);
} // namespace entity::shared

#endif // SHARED_ENTITY_FACTORY_HH
