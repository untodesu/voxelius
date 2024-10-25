// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2024, Voxelius Contributors
#pragma once
#include <enet/enet.h>
#include <game/shared/globals.hh>

class Config;

namespace globals
{
extern Config server_config;

extern ENetHost *server_host;

extern bool is_running;
extern unsigned int tickrate;
extern std::uint64_t tickrate_dt;
} // namespace globals
