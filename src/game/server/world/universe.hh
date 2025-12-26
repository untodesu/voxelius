#pragma once

#include "shared/types.hh"

class Chunk;
class Dimension;

struct Session;

namespace universe
{
void init(void);
void init_late(void);
void shutdown(void);
} // namespace universe

namespace universe
{
Chunk* load_chunk(Dimension* dimension, const chunk_pos& cpos);
void save_chunk(Dimension* dimension, const chunk_pos& cpos);
void save_all_chunks(Dimension* dimension);
} // namespace universe
