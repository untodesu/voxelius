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

        auto albedo_still = block_atlas::find(def.albedo_still);
        auto albedo_flowing = block_atlas::find(def.albedo_flowing);

        if(albedo_still == nullptr) {
            LOG_WARNING("fluid {}: failed to load still textures", id);
            continue;
        }

        if(albedo_flowing == nullptr) {
            LOG_WARNING("fluid {}: failed to load flowing textures", id);
            continue;
        }

        CachedFluid cached {};
        cached.albedo_still = albedo_still;
        cached.albedo_flowing = albedo_flowing;
        cached.mask_still = nullptr;
        cached.mask_flowing = nullptr;

        if(def.mask_still.has_value()) {
            std::array mask_frames { def.mask_still.value() };

            cached.mask_still = block_atlas::find(mask_frames);

            if(cached.mask_still == nullptr) {
                LOG_WARNING("fluid {}: failed to load still mask", id);
            }
        }

        if(def.mask_flowing.has_value()) {
            std::array mask_frames { def.mask_flowing.value() };

            cached.mask_flowing = block_atlas::find(mask_frames);

            if(cached.mask_flowing == nullptr) {
                LOG_WARNING("fluid {}: failed to load flowing mask", id);
            }
        }

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
