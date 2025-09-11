#include "client/pch.hh"

#include "client/sound/sound.hh"

#include "core/config/number.hh"
#include "core/io/config_map.hh"
#include "core/math/constexpr.hh"
#include "core/resource/resource.hh"

#include "shared/world/dimension.hh"

#include "shared/coord.hh"
#include "shared/protocol.hh"

#include "client/entity/camera.hh"
#include "client/entity/sound_emitter.hh"
#include "client/gui/settings.hh"
#include "client/resource/sound_effect.hh"

#include "client/const.hh"
#include "client/globals.hh"
#include "client/session.hh"

config::Float sound::volume_master(100.0f, 0.0f, 100.0f);
config::Float sound::volume_effects(100.0f, 0.0f, 100.0f);
config::Float sound::volume_music(100.0f, 0.0f, 100.0f);
config::Float sound::volume_ui(100.0f, 0.0f, 100.0f);

static ALuint generic_source;
static ALuint player_source;
static ALuint ui_source;

static resource_ptr<SoundEffect> sfx_generic;
static resource_ptr<SoundEffect> sfx_player;
static resource_ptr<SoundEffect> sfx_ui;

void sound::init_config(void)
{
    globals::client_config.add_value("sound.volume_master", sound::volume_master);
    globals::client_config.add_value("sound.volume_effects", sound::volume_effects);
    globals::client_config.add_value("sound.volume_music", sound::volume_music);
    globals::client_config.add_value("sound.volume_ui", sound::volume_ui);

    settings::add_slider(1, sound::volume_master, settings_location::SOUND, "sound.volume_master", false, "%.0f%%");

    settings::add_slider(0, sound::volume_effects, settings_location::SOUND_LEVELS, "sound.volume_effects", false, "%.0f%%");
    settings::add_slider(1, sound::volume_music, settings_location::SOUND_LEVELS, "sound.volume_music", false, "%.0f%%");
    settings::add_slider(2, sound::volume_ui, settings_location::SOUND_LEVELS, "sound.volume_ui", false, "%.0f%%");
}

void sound::init(void)
{
    alGenSources(1, &generic_source);
    alSourcei(generic_source, AL_SOURCE_RELATIVE, AL_TRUE);
    alSource3f(generic_source, AL_POSITION, 0.0f, 0.0f, 0.0f);
    alSource3f(generic_source, AL_VELOCITY, 0.0f, 0.0f, 0.0f);

    alGenSources(1, &player_source);
    alSourcei(player_source, AL_SOURCE_RELATIVE, AL_TRUE);
    alSource3f(player_source, AL_POSITION, 0.0f, 0.0f, 0.0f);
    alSource3f(player_source, AL_VELOCITY, 0.0f, 0.0f, 0.0f);

    alGenSources(1, &ui_source);
    alSourcei(ui_source, AL_SOURCE_RELATIVE, AL_TRUE);
    alSource3f(ui_source, AL_POSITION, 0.0f, 0.0f, 0.0f);
    alSource3f(ui_source, AL_VELOCITY, 0.0f, 0.0f, 0.0f);

    sfx_generic = nullptr;
    sfx_player = nullptr;
    sfx_ui = nullptr;
}

void sound::init_late(void)
{
}

void sound::shutdown(void)
{
    sfx_ui = nullptr;
    sfx_player = nullptr;
    sfx_generic = nullptr;

    alDeleteBuffers(1, &ui_source);
    alDeleteSources(1, &generic_source);
    alDeleteSources(1, &player_source);
}

void sound::update(void)
{
    auto effects_gain = math::clamp(0.01f * sound::volume_effects.get_value(), 0.0f, 1.0f);
    alSourcef(generic_source, AL_GAIN, effects_gain);
    alSourcef(player_source, AL_GAIN, effects_gain);

    auto ui_gain = math::clamp(0.01f * sound::volume_ui.get_value(), 0.0f, 1.0f);
    alSourcef(ui_source, AL_GAIN, ui_gain);
}

void sound::play_generic(std::string_view sound, bool looping, float pitch)
{
    if(sound.size()) {
        sound::play_generic(resource::load<SoundEffect>(sound), looping, pitch);
    }
    else {
        sound::play_generic(static_cast<resource_ptr<SoundEffect>>(nullptr), looping, pitch);
    }
}

void sound::play_entity(entt::entity entity, std::string_view sound, bool looping, float pitch)
{
    if(sound.size()) {
        sound::play_entity(entity, resource::load<SoundEffect>(sound), looping, pitch);
    }
    else {
        sound::play_entity(entity, static_cast<resource_ptr<SoundEffect>>(nullptr), looping, pitch);
    }
}

void sound::play_player(std::string_view sound, bool looping, float pitch)
{
    if(sound.size()) {
        sound::play_player(resource::load<SoundEffect>(sound), looping, pitch);
    }
    else {
        sound::play_player(static_cast<resource_ptr<SoundEffect>>(nullptr), looping, pitch);
    }
}

void sound::play_ui(std::string_view sound, bool looping, float pitch)
{
    if(sound.size()) {
        sound::play_ui(resource::load<SoundEffect>(sound), looping, pitch);
    }
    else {
        sound::play_ui(static_cast<resource_ptr<SoundEffect>>(nullptr), looping, pitch);
    }
}

void sound::play_generic(resource_ptr<SoundEffect> sound, bool looping, float pitch)
{
    alSourceRewind(generic_source);

    sfx_generic = sound;

    if(sfx_generic) {
        alSourcei(generic_source, AL_BUFFER, sfx_generic->buffer);
        alSourcei(generic_source, AL_LOOPING, looping);
        alSourcef(generic_source, AL_PITCH, math::clamp(pitch, MIN_PITCH, MAX_PITCH));
        alSourcePlay(generic_source);
    }
}

void sound::play_entity(entt::entity entity, resource_ptr<SoundEffect> sound, bool looping, float pitch)
{
    if(globals::dimension && globals::dimension->entities.valid(entity)) {
        if(auto emitter = globals::dimension->entities.try_get<entity::SoundEmitter>(entity)) {
            alSourceRewind(emitter->source);

            emitter->sound = sound;

            if(emitter->sound) {
                alSourcei(emitter->source, AL_BUFFER, emitter->sound->buffer);
                alSourcei(emitter->source, AL_LOOPING, looping);
                alSourcef(emitter->source, AL_PITCH, math::clamp(pitch, MIN_PITCH, MAX_PITCH));
                alSourcePlay(emitter->source);
            }
        }
    }
}

void sound::play_player(resource_ptr<SoundEffect> sound, bool looping, float pitch)
{
    if(sound && session::is_ingame()) {
        protocol::EntitySound packet;
        packet.entity = globals::player;
        packet.sound = sound->name;
        packet.looping = looping;
        packet.pitch = pitch;

        protocol::send(session::peer, protocol::encode(packet));
    }

    alSourceRewind(player_source);

    sfx_player = sound;

    if(sfx_player) {
        alSourcei(player_source, AL_BUFFER, sfx_player->buffer);
        alSourcei(player_source, AL_LOOPING, looping);
        alSourcef(player_source, AL_PITCH, math::clamp(pitch, MIN_PITCH, MAX_PITCH));
        alSourcePlay(player_source);
    }
}

void sound::play_ui(resource_ptr<SoundEffect> sound, bool looping, float pitch)
{
    alSourceRewind(ui_source);

    sfx_ui = sound;

    if(sfx_ui) {
        alSourcei(ui_source, AL_BUFFER, sfx_ui->buffer);
        alSourcei(ui_source, AL_LOOPING, looping);
        alSourcef(ui_source, AL_PITCH, math::clamp(pitch, MIN_PITCH, MAX_PITCH));
        alSourcePlay(ui_source);
    }
}
