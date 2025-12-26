// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: velocity.cc
// Description: Velocity component

#include "shared/pch.hh"

#include "shared/entity/velocity.hh"

#include "shared/entity/stasis.hh"
#include "shared/entity/transform.hh"

#include "shared/world/dimension.hh"

#include "shared/globals.hh"

void Velocity::fixed_update(Dimension* dimension)
{
    auto group = dimension->entities.group<Velocity>(entt::get<Transform>, entt::exclude<Stasis>);

    for(auto [entity, velocity, transform] : group.each()) {
        transform.local += velocity.value * globals::fixed_frametime;
    }
}
