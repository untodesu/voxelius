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
