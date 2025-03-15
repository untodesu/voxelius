#ifndef CLIENT_SOUND_HH
#define CLIENT_SOUND_HH 1
#pragma once

#include "core/resource.hh"

class ConfigFloat;
struct SoundEffect;

namespace sound
{
extern ConfigFloat volume_master;
extern ConfigFloat volume_effects;
extern ConfigFloat volume_music;
extern ConfigFloat volume_ui;
} // namespace sound

namespace sound
{
void init(void);
void init_late(void);
void deinit(void);
void update(void);
} // namespace sound

namespace sound
{
void play_generic(const char *sound, bool looping, float pitch);
void play_entity(entt::entity entity, const char *sound, bool looping, float pitch);
void play_player(const char *sound, bool looping, float pitch);
void play_ui(const char *sound, bool looping, float pitch);
} // namespace sound

namespace sound
{
void play_generic(resource_ptr<SoundEffect> sound, bool looping, float pitch);
void play_entity(entt::entity entity, resource_ptr<SoundEffect> sound, bool looping, float pitch);
void play_player(resource_ptr<SoundEffect> sound, bool looping, float pitch);
void play_ui(resource_ptr<SoundEffect> sound, bool looping, float pitch);
} // namespace sound

#endif /* CLIENT_SOUND_HH  */
