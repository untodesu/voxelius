// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: random_tick.hh
// Description: Voxel random ticking

#ifndef SERVER_WORLD_RANDOM_TICK_HH
#define SERVER_WORLD_RANDOM_TICK_HH
#pragma once

#include "shared/types.hh"

class Chunk;

namespace random_tick
{
void init(void);
void tick(const chunk_pos& cpos, Chunk* chunk);
} // namespace random_tick

#endif
