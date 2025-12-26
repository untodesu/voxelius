#pragma once

#include "shared/types.hh"

class Dimension;

struct Session;

namespace worldgen
{
void init(void);
} // namespace worldgen

namespace worldgen
{
bool is_generating(Dimension* dimension, const chunk_pos& cpos);
void request_chunk(Session* session, const chunk_pos& cpos);
} // namespace worldgen
