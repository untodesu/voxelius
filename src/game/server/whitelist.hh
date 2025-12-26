// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: whitelist.hh
// Description: Player allow-list

#ifndef SERVER_WHITELIST_HH
#define SERVER_WHITELIST_HH
#pragma once

namespace config
{
class Boolean;
class String;
} // namespace config

namespace whitelist
{
extern config::Boolean enabled;
extern config::String filename;
} // namespace whitelist

namespace whitelist
{
void init(void);
void init_late(void);
void shutdown(void);
} // namespace whitelist

namespace whitelist
{
bool contains(std::string_view username);
bool matches(std::string_view username, std::uint64_t password_hash);
} // namespace whitelist

#endif
