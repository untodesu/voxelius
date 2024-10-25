// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2024, Voxelius Contributors
#pragma once
#include <mathlib/vec3angles.hh>

struct HeadComponent final {
    Vec3angles angles {};
    Vec3f offset {};
};
