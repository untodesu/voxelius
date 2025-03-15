#include "server/pch.hh"
#include "server/globals.hh"

#include "core/config.hh"

#include "shared/protocol.hh"

Config globals::server_config;

ENetHost *globals::server_host;

bool globals::is_running;
unsigned int globals::tickrate;
std::uint64_t globals::tickrate_dt;

Dimension *globals::spawn_dimension;
std::unordered_map<std::string, Dimension *> globals::dimensions;
