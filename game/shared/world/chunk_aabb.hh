#ifndef SHARED_CHUNK_AABB
#define SHARED_CHUNK_AABB 1
#pragma once

#include "core/math/aabb.hh"

#include "shared/types.hh"

namespace world
{
using ChunkAABB = math::AABB<chunk_pos::value_type>;
} // namespace world

#endif // SHARED_CHUNK_AABB
