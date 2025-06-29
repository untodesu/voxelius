#ifndef SHARED_FACTORY
#define SHARED_FACTORY 1
#pragma once

class Dimension;

namespace shared_factory
{
void create_player(Dimension* dimension, entt::entity entity);
} // namespace shared_factory

#endif // SHARED_FACTORY
