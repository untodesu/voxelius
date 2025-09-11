#pragma once

struct SoundEffect final {
    static void register_resource(void);

    std::string name;
    ALuint buffer;
};
