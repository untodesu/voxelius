#ifndef CLIENT_CHUNK_MESHER_HH
#define CLIENT_CHUNK_MESHER_HH 1
#pragma once

#include "client/chunk_vbo.hh"

struct ChunkMeshComponent final {
    std::vector<ChunkVBO> quad_nb;
    std::vector<ChunkVBO> quad_b;
};

namespace chunk_mesher
{
void init(void);
void deinit(void);
void update(void);
} // namespace chunk_mesher

#endif // CLIENT_CHUNK_MESHER_HH
