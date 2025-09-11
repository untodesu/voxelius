#include "server/pch.hh"

#include "server/globals.hh"

#include "core/io/config_map.hh"

#include "shared/protocol.hh"

io::ConfigMap globals::server_config;

ENetHost* globals::server_host;

bool globals::is_running;
unsigned int globals::tickrate;
std::uint64_t globals::tickrate_dt;

world::Dimension* globals::spawn_dimension;
std::unordered_map<std::string, world::Dimension*> globals::dimensions;
