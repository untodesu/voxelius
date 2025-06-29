#ifndef CLIENT_SOUND_EMITTER_HH
#define CLIENT_SOUND_EMITTER_HH 1
#pragma once

#include "core/resource.hh"

struct SoundEffect;

struct SoundEmitterComponent final {
    resource_ptr<SoundEffect> sound;
    ALuint source;

public:
    explicit SoundEmitterComponent(void);
    virtual ~SoundEmitterComponent(void);

public:
    static void update(void);
};

#endif /* CLIENT_SOUND_EMITTER_HH  */
