// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: chunk_aabb.hh
// Description: Axis-aligned bounding box for chunk-domain coordinates

#ifndef SHARED_WORLD_CHUNK_AABB_HH
#define SHARED_WORLD_CHUNK_AABB_HH
#pragma once

#include "core/math/aabb.hh"

#include "shared/types.hh"

using ChunkAABB = math::AABB<chunk_pos::value_type>;

#endif
