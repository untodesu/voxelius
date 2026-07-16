#include "shared/pch.hh"

#include "shared/world/biome_registry.hh"

#include "shared/mod_context.hh"

static std::vector<BiomeDefinition> s_definitions;
static emhash8::HashMap<Identifier, biome_id_type> s_names;
static emhash8::HashMap<biome_id_type, Identifier> s_reverse_names;

std::span<const BiomeDefinition> biome_registry::all_definitions(void)
{
    return s_definitions;
}

void biome_registry::commit(ModContext& ctx)
{
    if(s_definitions.empty()) {
        s_definitions.emplace_back();
    }

    auto biomes = ctx.take_biomes();
    auto names = ctx.take_biome_names();

    biome_id_type biome_offset;

    if(biomes.empty()) {
        biome_offset = static_cast<biome_id_type>(s_definitions.size());
    }
    else {
        biome_offset = static_cast<biome_id_type>(s_definitions.size()) - 1;
    }

    for(auto& def : biomes) {
        s_definitions.emplace_back(std::move(def));
    }

    for(auto& [name, id] : names) {
        id += biome_offset;
        s_names.emplace(std::move(name), id);
        s_reverse_names.emplace(id, std::move(name));
    }
}

void biome_registry::purge(void)
{
    s_definitions.clear();
    s_names.clear();
    s_reverse_names.clear();
}

biome_id_type biome_registry::find(const Identifier& id)
{
    auto it = s_names.find(id);

    if(it == s_names.cend())
        return BIOME_ID_NULL;
    return it->second;
}

std::optional<Identifier> biome_registry::name_of(biome_id_type id)
{
    auto it = s_reverse_names.find(id);

    if(it == s_reverse_names.cend()) {
        return std::nullopt;
    }

    return it->second;
}
