#pragma once

#include "client/world/chunk_vbo.hh"

namespace world
{
struct ChunkMesh final {
    std::vector<ChunkVBO> quad_nb;
    std::vector<ChunkVBO> quad_b;
};
} // namespace world

namespace world::chunk_mesher
{
void init(void);
void shutdown(void);
void update(void);
} // namespace world::chunk_mesher
