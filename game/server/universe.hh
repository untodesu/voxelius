#ifndef SERVER_UNIVERSE_HH
#define SERVER_UNIVERSE_HH 1
#pragma once

#include "shared/types.hh"

class Chunk;
class Dimension;
class Session;

namespace universe
{
void init(void);
void init_late(void);
void deinit(void);
} // namespace universe

namespace universe
{
Chunk* load_chunk(Dimension* dimension, const chunk_pos& cpos);
void save_chunk(Dimension* dimension, const chunk_pos& cpos);
void save_all_chunks(Dimension* dimension);
} // namespace universe

#endif /* SERVER_UNIVERSE_HH  */
