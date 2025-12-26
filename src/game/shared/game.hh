// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: game.hh
// Description: Shared initialization logic

#ifndef SHARED_GAME_HH
#define SHARED_GAME_HH
#pragma once

namespace shared_game
{
void init(int argc, char** argv, std::string_view logfile = {});
void shutdown(void);
} // namespace shared_game

#endif
