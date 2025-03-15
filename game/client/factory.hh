#ifndef CLIENT_FACTORY_HH
#define CLIENT_FACTORY_HH 1
#pragma once

class Dimension;

namespace client_factory
{
void create_player(Dimension *dimension, entt::entity entity);
} // namespace client_factory

#endif /* CLIENT_FACTORY_HH */
