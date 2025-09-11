#include "core/pch.hh"

#include "core/resource/resource.hh"

struct ResourceLoader final {
    ResourceLoadFunc load_func;
    ResourceFreeFunc free_func;
    emhash8::HashMap<std::string, std::shared_ptr<const void>> resources;
    std::vector<std::shared_ptr<const void>> cache;
    std::string class_name;
};

namespace
{
emhash8::HashMap<std::type_index, std::unique_ptr<ResourceLoader>> loaders;
} // namespace

void resource::detail::register_loader(const std::type_info& type, ResourceLoadFunc load_func, ResourceFreeFunc free_func)
{
    assert(load_func);
    assert(free_func);

    auto type_index = std::type_index(type);
    auto loader = std::make_unique<ResourceLoader>();
    loader->class_name = type.name();
    loader->load_func = load_func;
    loader->free_func = free_func;

    assert(!loaders.contains(type_index));

    loaders.insert_or_assign(type_index, std::move(loader));
}

std::shared_ptr<const void> resource::detail::load_resource(const std::type_info& type, std::string_view name, std::uint32_t flags)
{
    auto name_str = std::string(name);
    auto type_index = std::type_index(type);
    auto loader = loaders.find(type_index);

    if(loader == loaders.cend()) {
        spdlog::error("resource: no loader registered for type [{}]", type.name());
        return nullptr;
    }

    auto resource_it = loader->second->resources.find(name_str);

    if(resource_it == loader->second->resources.cend()) {
        auto resource_raw = loader->second->load_func(name_str.c_str(), flags);

        if(resource_raw == nullptr) {
            spdlog::error("resource: {} [{}]: load failed", loader->second->class_name, name);
            return nullptr;
        }

        std::shared_ptr<const void> resource_ptr(resource_raw, [](const void* ptr) { /* empty */ });
        auto loaded_it = loader->second->resources.insert_or_assign(name_str, std::move(resource_ptr));

        if(flags & RESOURCE_CACHE) {
            loader->second->cache.push_back(loaded_it.first->second);
        }

        return loaded_it.first->second;
    }

    return resource_it->second;
}

std::shared_ptr<const void> resource::detail::find_resource(const std::type_info& type, std::string_view name)
{
    auto name_str = std::string(name);
    auto type_index = std::type_index(type);
    auto loader = loaders.find(type_index);

    if(loader == loaders.cend()) {
        spdlog::error("resource: no loader registered for type [{}]", type.name());
        return nullptr;
    }

    auto resource_it = loader->second->resources.find(name_str);

    if(resource_it == loader->second->resources.cend()) {
        spdlog::error("resource: {} [{}]: not found", loader->second->class_name, name);
        return nullptr;
    }

    return resource_it->second;
}

void resource::hard_cleanup(void)
{
    for(auto& [type_index, loader] : loaders) {
        loader->cache.clear();

        for(auto& [name, resource_ptr] : loader->resources) {
            if(resource_ptr.use_count() > 1) {
                spdlog::warn("resource: zombie resource: {} [{}] [use_count={}]", name, loader->class_name, resource_ptr.use_count());
            }
            else {
                spdlog::debug("resource: releasing {} [{}]", name, loader->class_name);
            }

            loader->free_func(resource_ptr.get());
        }

        loader->resources.clear();
    }

    loaders.clear();
}

void resource::soft_cleanup(void)
{
    for(auto& [type_index, loader] : loaders) {
        auto resource_it = loader->resources.begin();

        while(resource_it != loader->resources.end()) {
            if(resource_it->second.use_count() <= 1) {
                spdlog::debug("resource: releasing {} [{}]", resource_it->first, loader->class_name);

                loader->free_func(resource_it->second.get());
                resource_it = loader->resources.erase(resource_it);

                continue;
            }

            resource_it = std::next(resource_it);
        }
    }
}
