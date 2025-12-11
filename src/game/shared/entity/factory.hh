#pragma once

namespace world
{
class Dimension;
} // namespace world

namespace entity::shared
{
void create_player(world::Dimension* dimension, entt::entity entity);
} // namespace entity::shared
