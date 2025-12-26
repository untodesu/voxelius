// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: player_move.hh
// Description: Player movement

#ifndef CLIENT_ENTITY_PLAYER_MOVE_HH
#define CLIENT_ENTITY_PLAYER_MOVE_HH
#pragma once

constexpr static float PMOVE_MAX_SPEED_AIR = 16.0f;
constexpr static float PMOVE_MAX_SPEED_GROUND = 8.0f;
constexpr static float PMOVE_ACCELERATION_AIR = 3.0f;
constexpr static float PMOVE_ACCELERATION_GROUND = 6.0f;
constexpr static float PMOVE_FRICTION_GROUND = 10.0f;
constexpr static float PMOVE_JUMP_FORCE = 0.275f;

namespace player_move
{
void init(void);
void fixed_update(void);
void update_late(void);
} // namespace player_move

#endif
