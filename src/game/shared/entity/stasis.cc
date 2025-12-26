// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: stasis.cc
// Description: Freeze entites that are in an unloaded chunk

#include "shared/pch.hh"

#include "shared/entity/stasis.hh"

#include "shared/entity/transform.hh"

#include "shared/world/dimension.hh"

void Stasis::fixed_update(Dimension* dimension)
{
    auto view = dimension->entities.view<Transform>();

    for(auto [entity, transform] : view.each()) {
        if(dimension->find_chunk(transform.chunk)) {
            dimension->entities.remove<Stasis>(entity);
        }
        else {
            dimension->entities.emplace_or_replace<Stasis>(entity);
        }
    }
}
