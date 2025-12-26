// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: grounded.hh
// Description: Flag components for entites that rest on the ground

#ifndef SHARED_ENTITY_GROUNDED_HH
#define SHARED_ENTITY_GROUNDED_HH
#pragma once

#include "shared/world/voxel.hh"

// Assigned to entities which are grounded
// according to the collision and gravity system
struct Grounded final {
    VoxelMaterial surface;
};

#endif
