#pragma once

namespace world
{
class Dimension;
} // namespace world

namespace entity::client
{
void create_player(world::Dimension* dimension, entt::entity entity);
} // namespace entity::client
