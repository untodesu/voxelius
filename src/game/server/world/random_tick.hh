#pragma once

#include "shared/types.hh"

class Chunk;

namespace random_tick
{
void init(void);
void tick(const chunk_pos& cpos, Chunk* chunk);
} // namespace random_tick
