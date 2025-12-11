#pragma once

#include "shared/types.hh"

namespace world
{
class Chunk;
} // namespace world

namespace world::random_tick
{
void init(void);
void tick(const chunk_pos& cpos, Chunk* chunk);
} // namespace world::random_tick
