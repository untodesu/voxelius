// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: globals.hh
// Description: Global variables

#ifndef SERVER_GLOBALS_HH
#define SERVER_GLOBALS_HH
#pragma once

#include "shared/globals.hh"

class ConfigMap;

class Dimension;

namespace globals
{
extern ConfigMap server_config;

extern ENetHost* server_host;

extern bool is_running;
extern unsigned int tickrate;
extern std::uint64_t tickrate_dt;

extern Dimension* spawn_dimension;
extern std::unordered_map<std::string, Dimension*> dimensions;
} // namespace globals

#endif
