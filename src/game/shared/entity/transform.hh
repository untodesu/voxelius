// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: transform.hh
// Description: Transform component

#ifndef SHARED_ENTITY_TRANSFORM_HH
#define SHARED_ENTITY_TRANSFORM_HH
#pragma once

#include "shared/types.hh"

class Dimension;

struct Transform {
    chunk_pos chunk;
    glm::fvec3 local;
    glm::fvec3 angles;

public:
    // Updates Transform values so that
    // the local translation field is always within
    // local coodrinates; [floating-point precision]
    static void fixed_update(Dimension* dimension);
};

namespace client
{
// Client-side only - interpolated and previous transform
struct TransformIntr final : public Transform {};
struct TransformPrev final : public Transform {};
} // namespace client

#endif
