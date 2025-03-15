#ifndef SERVER_GLOBALS_HH
#define SERVER_GLOBALS_HH 1
#pragma once

#include "shared/globals.hh"

class Config;

class Dimension;

namespace globals
{
extern Config server_config;

extern ENetHost *server_host;

extern bool is_running;
extern unsigned int tickrate;
extern std::uint64_t tickrate_dt;

extern Dimension *spawn_dimension;
extern std::unordered_map<std::string, Dimension *> dimensions;
} // namespace globals

#endif /* SERVER_GLOBALS_HH  */
