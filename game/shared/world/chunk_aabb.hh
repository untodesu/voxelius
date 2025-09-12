#pragma once

#include "core/math/aabb.hh"

#include "shared/types.hh"

namespace world
{
using ChunkAABB = math::AABB<chunk_pos::value_type>;
} // namespace world
