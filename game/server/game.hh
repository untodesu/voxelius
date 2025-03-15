#ifndef SERVER_GAME_HH
#define SERVER_GAME_HH 1
#pragma once

class ConfigUnsigned;

namespace server_game
{
extern ConfigUnsigned view_distance;
} // namespace server_game

namespace server_game
{
extern std::uint64_t password_hash;
} // namespace server_game

namespace server_game
{
void init(void);
void init_late(void);
void deinit(void);
void fixed_update(void);
void fixed_update_late(void);
} // namespace server_game

#endif /* SERVER_GAME_HH  */
