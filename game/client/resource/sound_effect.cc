#include "client/pch.hh"

#include "client/resource/sound_effect.hh"

#include "core/resource/resource.hh"

#include "client/globals.hh"

static emhash8::HashMap<std::string, resource_ptr<SoundEffect>> resource_map;

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

template<>
resource_ptr<SoundEffect> resource::load<SoundEffect>(const char* name, unsigned int flags)
{
    auto it = resource_map.find(name);

    if(it != resource_map.cend()) {
        // Return an existing resource
        return it->second;
    }

    if(globals::sound_ctx == nullptr) {
        // Sound is disabled
        return nullptr;
    }

    auto file = PHYSFS_openRead(name);

    if(file == nullptr) {
        spdlog::warn("resource: {} [SoundEffect]: {}", name, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
        return nullptr;
    }

    drwav wav_info;

    if(!drwav_init(&wav_info, &drwav_read_physfs, &drwav_seek_physfs, file, nullptr)) {
        spdlog::warn("resource: {} [SoundEffect]: drwav_init failed", name);
        PHYSFS_close(file);
        return nullptr;
    }

    if(wav_info.channels != 1) {
        spdlog::warn("resource: {} [SoundEffect]: only mono sound files are allowed", name);
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

    auto new_resource = std::make_shared<SoundEffect>();
    new_resource->name = std::string(name);

    alGenBuffers(1, &new_resource->buffer);
    alBufferData(new_resource->buffer, AL_FORMAT_MONO16, samples, length, sample_rate);

    delete[] samples;

    return resource_map.insert_or_assign(name, new_resource).first->second;
}

template<>
void resource::hard_cleanup<SoundEffect>(void)
{
    for(const auto& it : resource_map) {
        if(it.second.use_count() > 1L) {
            spdlog::warn("resource: zombie resource [SoundEffect] {} [use_count={}]", it.first, it.second.use_count());
        }
        else {
            spdlog::debug("resource: releasing [SoundEffect] {}", it.first);
        }

        alDeleteBuffers(1, &it.second->buffer);
    }

    resource_map.clear();
}

template<>
void resource::soft_cleanup<SoundEffect>(void)
{
    auto iter = resource_map.cbegin();

    while(iter != resource_map.cend()) {
        if(iter->second.use_count() == 1L) {
            spdlog::debug("resource: releasing [SoundEffect] {}", iter->first);

            alDeleteBuffers(1, &iter->second->buffer);

            iter = resource_map.erase(iter);

            continue;
        }

        iter = std::next(iter);
    }
}
