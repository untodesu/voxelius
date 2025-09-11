#pragma once

namespace config
{
class Unsigned;
} // namespace config

namespace server_game
{
extern config::Unsigned view_distance;
} // namespace server_game

namespace server_game
{
extern std::uint64_t password_hash;
} // namespace server_game

namespace server_game
{
void init(void);
void init_late(void);
void shutdown(void);
void fixed_update(void);
void fixed_update_late(void);
} // namespace server_game
