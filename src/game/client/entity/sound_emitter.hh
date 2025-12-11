#pragma once

#include "core/resource/resource.hh"

struct SoundEffect;

namespace entity
{
struct SoundEmitter final {
    resource_ptr<SoundEffect> sound;
    ALuint source;

public:
    explicit SoundEmitter(void);
    virtual ~SoundEmitter(void);

public:
    static void update(void);
};
} // namespace entity
