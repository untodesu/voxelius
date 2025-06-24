#include "client/pch.hh"

#include "client/sound_emitter.hh"

#include "core/config.hh"
#include "core/constexpr.hh"

#include "shared/coord.hh"
#include "shared/dimension.hh"
#include "shared/transform.hh"
#include "shared/velocity.hh"

#include "client/camera.hh"
#include "client/globals.hh"
#include "client/sound.hh"

SoundEmitterComponent::SoundEmitterComponent(void)
{
    alGenSources(1, &source);
    sound = nullptr;
}

SoundEmitterComponent::~SoundEmitterComponent(void)
{
    alSourceStop(source);
    alDeleteSources(1, &source);
}

void SoundEmitterComponent::update(void)
{
    if(globals::dimension) {
        const auto view = globals::dimension->entities.view<SoundEmitterComponent>();

        const auto& pivot = camera::position_chunk;
        const auto gain = cxpr::clamp(sound::volume_effects.get_value() * 0.01f, 0.0f, 1.0f);

        for(const auto [entity, emitter] : view.each()) {
            alSourcef(emitter.source, AL_GAIN, gain);

            if(const auto transform = globals::dimension->entities.try_get<TransformComponentIntr>(entity)) {
                auto position = coord::to_relative(pivot, transform->chunk, transform->local);
                alSource3f(emitter.source, AL_POSITION, position.x, position.y, position.z);
            }

            if(const auto velocity = globals::dimension->entities.try_get<VelocityComponent>(entity)) {
                alSource3f(emitter.source, AL_VELOCITY, velocity->value.x, velocity->value.y, velocity->value.z);
            }

            ALint source_state;
            alGetSourcei(emitter.source, AL_SOURCE_STATE, &source_state);

            if(source_state == AL_STOPPED) {
                alSourceRewind(emitter.source);
                emitter.sound = nullptr;
            }
        }
    }
}
