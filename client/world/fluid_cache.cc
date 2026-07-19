#include "client/pch.hh"

#include "client/world/fluid_cache.hh"

#include "shared/world/fluid_registry.hh"

#include "client/world/block_atlas.hh"

static std::vector<std::optional<CachedFluid>> s_cache;

void fluid_cache::init_late(void)
{
    auto definitions = fluid_registry::all_definitions();

    s_cache.clear();
    s_cache.resize(definitions.size());

    for(fluid_id_type id = 0; id < definitions.size(); id += 1) {
        if(id == FLUID_ID_NULL) {
            continue;
        }

        const auto& def = definitions[id];

        auto still = block_atlas::load(def.still_textures);
        auto flowing = block_atlas::load(def.flowing_textures);

        if(still == nullptr) {
            LOG_WARNING("fluid {}: failed to load still textures", id);
            continue;
        }

        if(flowing == nullptr) {
            LOG_WARNING("fluid {}: failed to load flowing textures", id);
            continue;
        }

        CachedFluid cached {};
        cached.still = still;
        cached.flowing = flowing;

        s_cache[id] = cached;
    }
}

void fluid_cache::shutdown(void)
{
    s_cache.clear();
}

const CachedFluid* fluid_cache::find(fluid_id_type id)
{
    if(id == FLUID_ID_NULL || id >= s_cache.size()) {
        return nullptr;
    }

    if(!s_cache[id].has_value()) {
        return nullptr;
    }

    return &s_cache[id].value();
}
