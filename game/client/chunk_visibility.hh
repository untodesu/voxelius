#ifndef CLIENT_CHUNK_VISIBILITY_HH
#define CLIENT_CHUNK_VISIBILITY_HH 1
#pragma once

#include "shared/types.hh"

struct ChunkVisibleComponent final {};

namespace chunk_visibility
{
void update_chunk(entt::entity entity);
void update_chunk(const chunk_pos &cpos);
void update_chunks(void);
} // namespace chunk_visibility

namespace chunk_visibility
{
void cleanup(void);
void update(void);
} // namespace chunk_visibility

#endif /* CLIENT_CHUNK_VISIBILITY_HH  */
