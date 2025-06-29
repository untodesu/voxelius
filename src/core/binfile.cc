#include "core/pch.hh"

#include "core/binfile.hh"

#include "core/resource.hh"

static emhash8::HashMap<std::string, resource_ptr<BinFile>> resource_map;

template<>
resource_ptr<BinFile> resource::load<BinFile>(const char* name, unsigned int flags)
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

    auto new_resource = std::make_shared<BinFile>();
    new_resource->size = PHYSFS_fileLength(file);
    new_resource->buffer = new std::byte[new_resource->size];

    PHYSFS_readBytes(file, new_resource->buffer, new_resource->size);
    PHYSFS_close(file);

    return resource_map.insert_or_assign(name, new_resource).first->second;
}

template<>
void resource::hard_cleanup<BinFile>(void)
{
    for(const auto& it : resource_map) {
        if(it.second.use_count() > 1L) {
            spdlog::warn("resource: zombie resource [BinFile] {} [use_count={}]", it.first, it.second.use_count());
        } else {
            spdlog::debug("resource: releasing [BinFile] {}", it.first);
        }

        delete[] it.second->buffer;
    }

    resource_map.clear();
}

template<>
void resource::soft_cleanup<BinFile>(void)
{
    auto iter = resource_map.cbegin();

    while(iter != resource_map.cend()) {
        if(iter->second.use_count() == 1L) {
            spdlog::debug("resource: releasing [BinFile] {}", iter->first);

            delete[] iter->second->buffer;

            iter = resource_map.erase(iter);

            continue;
        }

        iter = std::next(iter);
    }
}
