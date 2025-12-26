// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: factory.hh
// Description: Boilerplate entity creation

#ifndef CLIENT_ENTITY_FACTORY_HH
#define CLIENT_ENTITY_FACTORY_HH
#pragma once

class Dimension;

namespace client
{
void create_player(Dimension* dimension, entt::entity entity);
} // namespace client

#endif
