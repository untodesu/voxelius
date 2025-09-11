#include "client/pch.hh"

#include "client/resource/sound_effect.hh"

#include "core/resource/resource.hh"

#include "core/utils/physfs.hh"

#include "client/globals.hh"

static std::size_t drwav_read_physfs(void* file, void* output, std::size_t count)
{
    return static_cast<std::size_t>(PHYSFS_readBytes(reinterpret_cast<PHYSFS_File*>(file), output, count));
}

static drwav_bool32 drwav_seek_physfs(void* file, int offset, drwav_seek_origin origin)
{
    if(origin == drwav_seek_origin_current) {
        return PHYSFS_seek(reinterpret_cast<PHYSFS_File*>(file), PHYSFS_tell(reinterpret_cast<PHYSFS_File*>(file)) + offset);
    }
    else {
        return PHYSFS_seek(reinterpret_cast<PHYSFS_File*>(file), offset);
    }
}

static const void* sound_effect_load_func(const char* name, std::uint32_t flags)
{
    assert(name);

    if(globals::sound_ctx == nullptr) {
        // Sound is disabled
        return nullptr;
    }

    auto file = PHYSFS_openRead(name);

    if(file == nullptr) {
        spdlog::warn("sfx: {}: {}", name, utils::physfs_error());
        return nullptr;
    }

    drwav wav_info;

    if(!drwav_init(&wav_info, &drwav_read_physfs, &drwav_seek_physfs, file, nullptr)) {
        spdlog::warn("sfx: {}: drwav_init failed", name);
        PHYSFS_close(file);
        return nullptr;
    }

    if(wav_info.channels != 1) {
        spdlog::warn("sfx: {}: only mono sound files are allowed", name);
        drwav_uninit(&wav_info);
        PHYSFS_close(file);
        return nullptr;
    }

    auto samples = new ALshort[wav_info.totalPCMFrameCount];
    auto count = drwav_read_pcm_frames_s16(&wav_info, wav_info.totalPCMFrameCount, reinterpret_cast<drwav_int16*>(samples));
    auto sample_rate = static_cast<ALsizei>(wav_info.sampleRate);
    auto length = static_cast<ALsizei>(count * sizeof(ALshort));

    drwav_uninit(&wav_info);
    PHYSFS_close(file);

    auto new_resource = new SoundEffect();
    new_resource->name = std::string(name);

    alGenBuffers(1, &new_resource->buffer);
    alBufferData(new_resource->buffer, AL_FORMAT_MONO16, samples, length, sample_rate);

    delete[] samples;

    return new_resource;
}

static void sound_effect_free_func(const void* resource)
{
    assert(resource);

    auto sound_effect = reinterpret_cast<const SoundEffect*>(resource);

    alDeleteBuffers(1, &sound_effect->buffer);

    delete sound_effect;
}

void SoundEffect::register_resource(void)
{
    resource::register_loader<SoundEffect>(&sound_effect_load_func, &sound_effect_free_func);
}
