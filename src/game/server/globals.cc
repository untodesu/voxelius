// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: globals.cc
// Description: Global variables

#include "server/pch.hh"

#include "server/globals.hh"

#include "core/io/config_map.hh"

#include "shared/protocol.hh"

ConfigMap globals::server_config;

ENetHost* globals::server_host;

bool globals::is_running;
unsigned int globals::tickrate;
std::uint64_t globals::tickrate_dt;

Dimension* globals::spawn_dimension;
std::unordered_map<std::string, Dimension*> globals::dimensions;
