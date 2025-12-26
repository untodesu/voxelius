// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: stasis.hh
// Description: Freeze entites that are in an unloaded chunk

#ifndef SHARED_ENTITY_STASIS_HH
#define SHARED_ENTITY_STASIS_HH
#pragma once

class Dimension;

// Attached to entities with transform values
// out of bounds in a specific dimension
struct Stasis final {
public:
    static void fixed_update(Dimension* dimension);
};

#endif
