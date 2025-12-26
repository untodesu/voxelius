// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: factory.hh
// Description: Boilerplate entity creation

#ifndef SHARED_ENTITY_FACTORY_HH
#define SHARED_ENTITY_FACTORY_HH
#pragma once

class Dimension;

namespace shared
{
void create_player(Dimension* dimension, entt::entity entity);
} // namespace shared

#endif
