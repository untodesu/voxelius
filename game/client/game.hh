#ifndef CLIENT_GAME_HH
#define CLIENT_GAME_HH 1
#pragma once

class ConfigBoolean;
class ConfigString;
class ConfigUnsigned;

namespace client_game
{
extern ConfigBoolean streamer_mode;
extern ConfigBoolean vertical_sync;
extern ConfigBoolean world_curvature;
extern ConfigUnsigned pixel_size;
extern ConfigUnsigned fog_mode;
extern ConfigString username;
} // namespace client_game

namespace client_game
{
extern bool hide_hud;
} // namespace client_game

namespace client_game
{
void init(void);
void init_late(void);
void deinit(void);
void fixed_update(void);
void fixed_update_late(void);
void update(void);
void update_late(void);
void render(void);
void layout(void);
} // namespace client_game

#endif /* CLIENT_GAME_HH */
