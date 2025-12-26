// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: worldgen.hh
// Description: Threaded requests for dimensions to generate chunks

#ifndef SERVER_WORLD_WORLDGEN_HH
#define SERVER_WORLD_WORLDGEN_HH
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

#endif
