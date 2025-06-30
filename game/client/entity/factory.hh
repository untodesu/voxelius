#ifndef CLIENT_FACTORY_HH
#define CLIENT_FACTORY_HH 1
#pragma once

namespace world
{
class Dimension;
} // namespace world

namespace entity::client
{
void create_player(world::Dimension* dimension, entt::entity entity);
} // namespace entity::client

#endif // CLIENT_FACTORY_HH
