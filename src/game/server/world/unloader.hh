// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: unloader.hh
// Description: Handle chunks that are out of view for all players

#ifndef SERVER_WORLD_UNLOADER_HH
#define SERVER_WORLD_UNLOADER_HH
#pragma once

class Dimension;

namespace unloader
{
void init(void);
void init_late(void);
void fixed_update_late(Dimension* dimension);
} // namespace unloader

#endif
