// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: gravity.hh
// Description: Gravity component

#ifndef SHARED_ENTITY_GRAVITY_HH
#define SHARED_ENTITY_GRAVITY_HH
#pragma once

class Dimension;

struct Gravity final {
public:
    static void fixed_update(Dimension* dimension);
};

#endif
