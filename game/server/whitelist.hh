#ifndef SERVER_WHITELIST_HH
#define SERVER_WHITELIST_HH 1
#pragma once

class ConfigBoolean;
class ConfigString;

namespace whitelist
{
extern ConfigBoolean enabled;
extern ConfigString filename;
} // namespace whitelist

namespace whitelist
{
void init(void);
void init_late(void);
void deinit(void);
} // namespace whitelist

namespace whitelist
{
bool contains(const char *username);
bool matches(const char *username, std::uint64_t password_hash);
} // namespace whitelist

#endif /* SERVER_WHITELIST_HH  */
