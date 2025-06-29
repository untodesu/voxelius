#ifndef CLIENT_SOUND_EFFECT_HH
#define CLIENT_SOUND_EFFECT_HH 1
#pragma once

struct SoundEffect final {
    std::string name;
    ALuint buffer;
};

#endif // CLIENT_SOUND_EFFECT_HH
