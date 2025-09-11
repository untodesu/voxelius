#pragma once

#include "shared/types.hh"

namespace world
{
class Chunk;
class Dimension;
} // namespace world

class Session;

namespace world::universe
{
void init(void);
void init_late(void);
void shutdown(void);
} // namespace world::universe

namespace world::universe
{
Chunk* load_chunk(Dimension* dimension, const chunk_pos& cpos);
void save_chunk(Dimension* dimension, const chunk_pos& cpos);
void save_all_chunks(Dimension* dimension);
} // namespace world::universe
