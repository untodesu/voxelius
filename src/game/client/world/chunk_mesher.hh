// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: chunk_mesher.hh
// Description: Threaded voxel mesh generation

#ifndef CLIENT_WORLD_CHUNK_MESHER_HH
#define CLIENT_WORLD_CHUNK_MESHER_HH
#pragma once

#include "client/world/chunk_vbo.hh"

struct ChunkMesh final {
    std::vector<ChunkVBO> quad_nb;
    std::vector<ChunkVBO> quad_b;
};

namespace chunk_mesher
{
void init(void);
void shutdown(void);
void update(void);
} // namespace chunk_mesher

#endif
