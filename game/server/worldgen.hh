#ifndef SERVER_WORLDGEN_HH
#define SERVER_WORLDGEN_HH 1
#pragma once

#include "shared/types.hh"

class Dimension;
class Session;

namespace worldgen
{
bool is_generating(Dimension *dimension, const chunk_pos &cpos);
void request_chunk(Session *session, const chunk_pos &cpos);
} // namespace worldgen

#endif /* SERVER_WORLDGEN_HH */
