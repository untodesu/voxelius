#include "shared/pch.hh"

#include "shared/biome_registry.hh"

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

    for(const auto& [name, local_id] : names) {
        auto global_id = local_id + biome_offset;
        auto [it, inserted] = s_names.try_emplace(name, global_id);

        if(!inserted) {
            LOG_WARNING("duplicate biome name: {}", name.full_string());
            continue;
        }

        s_reverse_names.try_emplace(global_id, name);
    }

    if(biomes.size()) {
        s_definitions.insert(s_definitions.end(), std::make_move_iterator(biomes.begin() + 1), std::make_move_iterator(biomes.end()));
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

const BiomeDefinition* biome_registry::find_definition(biome_id_type id)
{
    if(id == BIOME_ID_NULL || id >= s_definitions.size())
        return nullptr;
    return &s_definitions[id];
}

const BiomeDefinition* biome_registry::find_definition(const Identifier& id)
{
    return find_definition(find(id));
}
