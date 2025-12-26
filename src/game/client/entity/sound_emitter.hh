// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: sound_emitter.hh
// Description: Component for entities that make sounds

#ifndef CLIENT_ENTITY_SOUND_EMITTER_HH
#define CLIENT_ENTITY_SOUND_EMITTER_HH
#pragma once

#include "core/resource/resource.hh"

struct SoundEffect;

struct SoundEmitter final {
    resource_ptr<SoundEffect> sound;
    ALuint source;

public:
    explicit SoundEmitter(void);
    virtual ~SoundEmitter(void);

public:
    static void update(void);
};

#endif
