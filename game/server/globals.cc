// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2024, Voxelius Contributors
#include <common/config.hh>
#include <game/server/globals.hh>

Config globals::server_config = {};

ENetHost *globals::server_host = {};

bool globals::is_running = false;
unsigned int globals::tickrate = 30U;
std::uint64_t globals::tickrate_dt = 0;
