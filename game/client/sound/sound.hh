#ifndef CLIENT_SOUND_HH
#define CLIENT_SOUND_HH 1
#pragma once

#include "core/resource/resource.hh"

namespace config
{
class Float;
} // namespace config

struct SoundEffect;

namespace sound
{
extern config::Float volume_master;
extern config::Float volume_effects;
extern config::Float volume_music;
extern config::Float volume_ui;
} // namespace sound

namespace sound
{
void init_config(void);
void init(void);
void init_late(void);
void shutdown(void);
void update(void);
} // namespace sound

namespace sound
{
void play_generic(std::string_view sound, bool looping, float pitch);
void play_entity(entt::entity entity, std::string_view sound, bool looping, float pitch);
void play_player(std::string_view sound, bool looping, float pitch);
void play_ui(std::string_view sound, bool looping, float pitch);
} // namespace sound

namespace sound
{
void play_generic(resource_ptr<SoundEffect> sound, bool looping, float pitch);
void play_entity(entt::entity entity, resource_ptr<SoundEffect> sound, bool looping, float pitch);
void play_player(resource_ptr<SoundEffect> sound, bool looping, float pitch);
void play_ui(resource_ptr<SoundEffect> sound, bool looping, float pitch);
} // namespace sound

#endif // CLIENT_SOUND_HH
