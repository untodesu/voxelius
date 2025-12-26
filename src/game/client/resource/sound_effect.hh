// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: sound_effect.hh
// Description: Sound effect resource

#ifndef CLIENT_RESOURCE_SOUND_EFFECT_HH
#define CLIENT_RESOURCE_SOUND_EFFECT_HH
#pragma once

struct SoundEffect final {
    static void register_resource(void);

    std::string name;
    ALuint buffer;
};

#endif
