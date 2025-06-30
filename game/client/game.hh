#ifndef CLIENT_GAME_HH
#define CLIENT_GAME_HH 1
#pragma once

namespace config
{
class Boolean;
class String;
class Unsigned;
} // namespace config

namespace client_game
{
extern config::Boolean streamer_mode;
extern config::Boolean vertical_sync;
extern config::Boolean world_curvature;
extern config::Unsigned fog_mode;
extern config::String username;
} // namespace client_game

namespace client_game
{
extern bool hide_hud;
} // namespace client_game

namespace client_game
{
void init(void);
void init_late(void);
void shutdown(void);
void fixed_update(void);
void fixed_update_late(void);
void update(void);
void update_late(void);
void render(void);
void layout(void);
} // namespace client_game

#endif // CLIENT_GAME_HH
