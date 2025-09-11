#pragma once

#include "shared/globals.hh"

namespace io
{
class ConfigMap;
} // namespace io

namespace world
{
class Dimension;
} // namespace world

namespace globals
{
extern io::ConfigMap server_config;

extern ENetHost* server_host;

extern bool is_running;
extern unsigned int tickrate;
extern std::uint64_t tickrate_dt;

extern world::Dimension* spawn_dimension;
extern std::unordered_map<std::string, world::Dimension*> dimensions;
} // namespace globals
