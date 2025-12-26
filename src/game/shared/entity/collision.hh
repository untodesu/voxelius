// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: collision.hh
// Description: Collision detection

#ifndef SHARED_ENTITY_COLLISION_HH
#define SHARED_ENTITY_COLLISION_HH
#pragma once

#include "core/math/aabb.hh"

class Dimension;

struct Collision final {
    math::AABBf aabb;

public:
    // NOTE: Collision::fixed_update must be called
    // before Transform::fixed_update and Velocity::fixed_update
    // because both transform and velocity may be updated internally
    static void fixed_update(Dimension* dimension);
};

#endif
