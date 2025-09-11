#pragma once

#include "shared/types.hh"

namespace world
{
class Dimension;
} // namespace world

class Session;

namespace world::worldgen
{
void init(void);
} // namespace world::worldgen

namespace world::worldgen
{
bool is_generating(Dimension* dimension, const chunk_pos& cpos);
void request_chunk(Session* session, const chunk_pos& cpos);
} // namespace world::worldgen
