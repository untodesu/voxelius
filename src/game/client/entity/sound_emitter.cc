#include "client/pch.hh"

#include "client/entity/sound_emitter.hh"

#include "core/config/number.hh"

#include "core/math/constexpr.hh"

#include "shared/entity/transform.hh"
#include "shared/entity/velocity.hh"

#include "shared/world/dimension.hh"

#include "shared/coord.hh"

#include "client/entity/camera.hh"

#include "client/sound/sound.hh"

#include "client/globals.hh"

SoundEmitter::SoundEmitter(void)
{
    alGenSources(1, &source);
    sound = nullptr;
}

SoundEmitter::~SoundEmitter(void)
{
    alSourceStop(source);
    alDeleteSources(1, &source);
}

void SoundEmitter::update(void)
{
    if(globals::dimension) {
        const auto view = globals::dimension->entities.view<SoundEmitter>();

        const auto& pivot = camera::position_chunk;
        const auto gain = glm::clamp(sound::volume_effects.get_value() * 0.01f, 0.0f, 1.0f);

        for(const auto [entity, emitter] : view.each()) {
            alSourcef(emitter.source, AL_GAIN, gain);

            if(const auto transform = globals::dimension->entities.try_get<client::TransformIntr>(entity)) {
                auto position = coord::to_relative(pivot, transform->chunk, transform->local);
                alSource3f(emitter.source, AL_POSITION, position.x, position.y, position.z);
            }

            if(const auto velocity = globals::dimension->entities.try_get<Velocity>(entity)) {
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
