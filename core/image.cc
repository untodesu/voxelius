#include "core/pch.hh"

#include "core/image.hh"

#include "core/resource.hh"

static emhash8::HashMap<std::string, resource_ptr<Image>> resource_map;

static int stbi_physfs_read(void* context, char* data, int size)
{
    return PHYSFS_readBytes(reinterpret_cast<PHYSFS_File*>(context), data, size);
}

static void stbi_physfs_skip(void* context, int count)
{
    auto file = reinterpret_cast<PHYSFS_File*>(context);
    PHYSFS_seek(file, PHYSFS_tell(file) + count);
}

static int stbi_physfs_eof(void* context)
{
    return PHYSFS_eof(reinterpret_cast<PHYSFS_File*>(context));
}

template<>
resource_ptr<Image> resource::load<Image>(const char* name, unsigned int flags)
{
    auto it = resource_map.find(name);

    if(it != resource_map.cend()) {
        // Return an existing resource
        return it->second;
    }

    auto file = PHYSFS_openRead(name);

    if(file == nullptr) {
        spdlog::warn("resource: {}: {}", name, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
        return nullptr;
    }

    if(flags & IMAGE_LOAD_FLIP) {
        stbi_set_flip_vertically_on_load(true);
    } else {
        stbi_set_flip_vertically_on_load(false);
    }

    stbi_io_callbacks callbacks;
    callbacks.read = &stbi_physfs_read;
    callbacks.skip = &stbi_physfs_skip;
    callbacks.eof = &stbi_physfs_eof;

    auto new_resource = std::make_shared<Image>();

    if(flags & IMAGE_LOAD_GRAY) {
        new_resource->pixels = stbi_load_from_callbacks(&callbacks, file, &new_resource->size.x, &new_resource->size.y, nullptr, STBI_grey);
    } else {
        new_resource->pixels = stbi_load_from_callbacks(
            &callbacks, file, &new_resource->size.x, &new_resource->size.y, nullptr, STBI_rgb_alpha);
    }

    PHYSFS_close(file);

    if(new_resource->pixels == nullptr) {
        spdlog::warn("resource: {}: {}", name, stbi_failure_reason());
        return nullptr;
    }

    if(new_resource->size.x <= 0 || new_resource->size.y <= 0) {
        spdlog::warn("resource {}: invalid dimensions", name);
        stbi_image_free(new_resource->pixels);
        return nullptr;
    }

    return resource_map.insert_or_assign(name, new_resource).first->second;
}

template<>
void resource::hard_cleanup<Image>(void)
{
    for(const auto& it : resource_map) {
        if(it.second.use_count() > 1L) {
            spdlog::warn("resource: zombie resource [Image] {} [use_count={}]", it.first, it.second.use_count());
        } else {
            spdlog::debug("resource: releasing [Image] {}", it.first);
        }

        stbi_image_free(it.second->pixels);
    }

    resource_map.clear();
}

template<>
void resource::soft_cleanup<Image>(void)
{
    auto iter = resource_map.cbegin();

    while(iter != resource_map.cend()) {
        if(iter->second.use_count() == 1L) {
            spdlog::debug("resource: releasing [Image] {}", iter->first);

            stbi_image_free(iter->second->pixels);

            iter = resource_map.erase(iter);

            continue;
        }

        iter = std::next(iter);
    }
}
