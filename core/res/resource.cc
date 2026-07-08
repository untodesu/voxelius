#include "core/pch.hh"

#include "core/res/resource.hh"

struct Loader final {
    res::load_func load_fn;
    res::free_func free_fn;

    emhash8::HashMap<std::string, res::handle<void>> resources;

    std::vector<res::handle<void>> cache;

    std::string classname;
};

static emhash8::HashMap<std::type_index, std::unique_ptr<Loader>> s_loaders;

void res::detail::register_loader(const std::type_info& type, load_func load_fn, free_func free_fn)
{
    assert(load_fn);
    assert(free_fn);

    auto loader = std::make_unique<Loader>();
    loader->classname = type.name();
    loader->load_fn = load_fn;
    loader->free_fn = free_fn;

    std::type_index type_index(type);

    assert(0 == s_loaders.count(type_index));

    LOG_DEBUG("registering new loader for <{}>", loader->classname);

    s_loaders.insert_or_assign(type_index, std::move(loader));
}

res::handle<void> res::detail::load_resource(const std::type_info& type, std::string_view path, std::uint32_t flags)
{
    std::string path_unfucked(path);
    std::type_index type_index(type);

    auto loader = s_loaders.find(type_index);

    if(loader == s_loaders.cend()) {
        LOG_WARNING("no loader present for <{}>", type.name());
        return nullptr;
    }

    auto found = loader->second->resources.find(path_unfucked);

    if(found == loader->second->resources.cend()) {
        auto raw = loader->second->load_fn(path_unfucked.c_str(), flags);

        if(raw == nullptr) {
            LOG_WARNING("{}<{}>: load failed", path_unfucked, type.name());
            return nullptr;
        }

        handle<void> resource(raw, [](const void* ptr) { /* empty */ });

        auto loaded = loader->second->resources.insert_or_assign(path_unfucked, std::move(resource));

        if(flags & RESFLAG_CACHE) {
            loader->second->cache.push_back(loaded.first->second);
        }

        return loaded.first->second;
    }

    return found->second;
}

res::handle<void> res::detail::find_resource(const std::type_info& type, std::string_view path)
{
    std::string path_unfucked(path);
    std::type_index type_index(type);

    auto loader = s_loaders.find(type_index);

    if(loader == s_loaders.cend()) {
        LOG_WARNING("no loader present for <{}>", type.name());
        return nullptr;
    }

    auto found = loader->second->resources.find(path_unfucked);

    if(found == loader->second->resources.cend()) {
        return nullptr;
    }

    return found->second;
}

void res::soft_purge(bool include_cached)
{
    for(auto& [type_index, loader] : s_loaders) {
        if(include_cached) {
            // Normally soft_purge is called after every
            // frame is rendered, but it also may be called
            // whenever we disconnect, so certain cached
            // resources might need to be freed; calling
            // hard_purge is also not an option specifically
            // because it doesn't care about use_count and
            // very unpleasant things (UAF) might happen
            loader->cache.clear();
        }

        auto iter = loader->resources.begin();

        while(iter != loader->resources.end()) {
            if(iter->second.use_count() <= 1) {
                LOG_DEBUG("releasing {}<{}>", iter->first, loader->classname);

                loader->free_fn(iter->second.get());

                iter = loader->resources.erase(iter);
            }
            else {
                iter = std::next(iter);
            }
        }
    }
}

void res::hard_purge(void)
{
    for(auto& [type_index, loader] : s_loaders) {
        loader->cache.clear();

        for(auto& [path, handle] : loader->resources) {
            if(handle.use_count() > 1) {
                LOG_WARNING("zombie resource: {}<{}> use_count={}", path, loader->classname, handle.use_count());
            }

            LOG_DEBUG("releasing {}<{}>", path, loader->classname);

            loader->free_fn(handle.get());
        }

        loader->resources.clear();
    }
}
