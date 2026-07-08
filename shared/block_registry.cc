#include "shared/pch.hh"

#include "shared/block_registry.hh"

#include "core/utils/crc64.hh"

#include "core/exception.hh"

#include "shared/mod_context.hh"

static std::vector<BlockDefinition> s_definitions;
static std::vector<BlockFamily> s_families;
static std::unordered_map<Identifier, block_id_type> s_names;
static std::unordered_map<block_id_type, Identifier> s_reverse_names;

BlockDefinition BlockOverridePatch::apply(BlockDefinition base, const BlockOverridePatch& patch) noexcept
{
    if(patch.render) {
        base.render = *patch.render;
    }

    if(patch.textures) {
        base.textures = *patch.textures;
    }

    if(patch.animated) {
        base.animated = *patch.animated;
    }

    if(patch.model_name) {
        base.model_name = *patch.model_name;
    }

    if(patch.model_offset) {
        base.model_offset = *patch.model_offset;
    }

    if(patch.bcoll_name) {
        base.bcoll_name = *patch.bcoll_name;
    }

    if(patch.bcoll_offset) {
        base.bcoll_offset = *patch.bcoll_offset;
    }

    if(patch.health) {
        base.health = *patch.health;
    }

    if(patch.sound_set) {
        base.sound_set = *patch.sound_set;
    }

    if(patch.emission) {
        base.emission = *patch.emission;
    }

    if(patch.dissipation) {
        base.dissipation = *patch.dissipation;
    }

    if(patch.touch) {
        base.touch = *patch.touch;
    }

    if(patch.touch_coeffs) {
        base.touch_coeffs = *patch.touch_coeffs;
    }

    if(patch.tags) {
        base.tags = *patch.tags;
    }

    if(patch.drops) {
        base.drops = *patch.drops;
        base.tools = BLOCK_TOOL_NONE;

        for(const auto& drop : base.drops) {
            base.tools = static_cast<block_tool_bit>(base.tools | drop.cond_tool_bits);
        }
    }

    return base;
}

blockstate_val_type BlockFamily::state_hash(std::string_view string) noexcept
{
    auto hash = static_cast<blockstate_val_type>(utils::crc64(string.data(), string.size()));
    state_values.insert_or_assign(hash, std::string(string));
    return hash;
}

std::string_view BlockFamily::state_value(blockstate_val_type value) noexcept
{
    auto it = state_values.find(value);

    if(it == state_values.cend()) {
        return {};
    }

    return it->second;
}

std::span<const BlockDefinition> block_registry::all_definitions(void) noexcept
{
    return s_definitions;
}

std::span<const BlockFamily> block_registry::all_families(void) noexcept
{
    return s_families;
}

void block_registry::commit(ModContext& ctx) noexcept
{
    if(s_definitions.empty()) {
        s_definitions.emplace_back();
    }

    if(s_families.empty()) {
        s_families.emplace_back();
    }

    auto blocks = ctx.take_blocks();
    auto families = ctx.take_block_families();
    auto names = ctx.take_block_names();

    block_id_type block_offset;
    block_family_id_type family_offset;

    if(blocks.empty()) {
        block_offset = static_cast<block_id_type>(s_definitions.size());
    }
    else {
        block_offset = static_cast<block_id_type>(s_definitions.size()) - 1;
    }

    if(families.empty()) {
        family_offset = static_cast<block_family_id_type>(s_families.size());
    }
    else {
        family_offset = static_cast<block_family_id_type>(s_families.size()) - 1;
    }

    for(auto& def : blocks) {
        if(def.family != BLOCK_FAMILY_ID_NULL) {
            def.family += family_offset;
        }
    }

    for(auto& family : families) {
        if(family.base_id != BLOCK_ID_NULL) {
            family.base_id += block_offset;
        }

        for(auto& [hash, id] : family.resolved_states) {
            id += block_offset;
        }
    }

    for(const auto& [name, local_id] : names) {
        auto global_id = local_id + block_offset;
        auto [it, inserted] = s_names.try_emplace(name, global_id);

        if(!inserted) {
            LOG_WARNING("block_registry: duplicate block name: {}", name.full_string());
            continue;
        }

        s_reverse_names.insert_or_assign(global_id, name);
    }

    if(!blocks.empty()) {
        s_definitions.insert(s_definitions.end(), std::make_move_iterator(blocks.begin() + 1), std::make_move_iterator(blocks.end()));
    }

    if(!families.empty()) {
        s_families.insert(s_families.end(), std::make_move_iterator(families.begin() + 1), std::make_move_iterator(families.end()));

        for(auto i = family_offset; i < s_families.size(); ++i) {
            const auto& family = s_families[i];

            for(const auto& rule : family.variants) {
                resolve_variant(family.base_id, rule.when);
            }
        }
    }
}

void block_registry::purge(void) noexcept
{
    s_definitions.clear();
    s_families.clear();
    s_names.clear();
    s_reverse_names.clear();
}

block_id_type block_registry::find(const Identifier& id) noexcept
{
    auto it = s_names.find(id);

    if(it == s_names.cend())
        return BLOCK_ID_NULL;
    return it->second;
}

std::optional<Identifier> block_registry::name_of(block_id_type id) noexcept
{
    auto it = s_reverse_names.find(id);

    if(it == s_reverse_names.cend()) {
        if(auto def = find_definition(id)) {
            if(auto family = find_family(def->family)) {
                return family->name;
            }
        }

        return std::nullopt;
    }

    return it->second;
}

const BlockDefinition* block_registry::find_definition(block_id_type id) noexcept
{
    if(id == BLOCK_ID_NULL || id >= s_definitions.size())
        return nullptr;
    return &s_definitions[id];
}

const BlockDefinition* block_registry::find_definition(const Identifier& id) noexcept
{
    return find_definition(find(id));
}

const BlockFamily* block_registry::find_family(block_family_id_type id) noexcept
{
    if(id == BLOCK_FAMILY_ID_NULL || id >= s_families.size())
        return nullptr;
    return &s_families[id];
}

const BlockFamily* block_registry::find_family(const Identifier& id) noexcept
{
    for(const auto& family : s_families) {
        if(family.name == id) {
            return &family;
        }
    }

    return nullptr;
}

bool block_registry::has_tag_all(block_id_type id, block_tag_bit tag_bits) noexcept
{
    if(const auto def = find_definition(id)) {
        return tag_bits == (def->tags & tag_bits);
    }

    return false;
}

bool block_registry::has_tag_any(block_id_type id, block_tag_bit tag_bits) noexcept
{
    if(const auto def = find_definition(id)) {
        return static_cast<bool>(def->tags & tag_bits);
    }

    return false;
}

block_id_type block_registry::resolve_variant(block_id_type curr_id,
    const std::unordered_map<blockstate_key_type, blockstate_val_type>& map) noexcept
{
    auto def = find_definition(curr_id);

    if(def == nullptr || def->family == BLOCK_FAMILY_ID_NULL) {
        return curr_id;
    }

    auto& family = s_families[def->family];

    std::vector<std::pair<blockstate_key_type, blockstate_val_type>> sorted(map.cbegin(), map.cend());
    std::sort(sorted.begin(), sorted.end(), [](const auto& a, const auto& b) noexcept {
        return a.first < b.first;
    });

    std::uint64_t hash = 0;

    for(const auto& it : sorted) {
        hash = utils::crc64(&it.first, sizeof(it.first), hash);
        hash = utils::crc64(&it.second, sizeof(it.second), hash);
    }

    if(family.resolved_states.contains(hash)) {
        return family.resolved_states.at(hash);
    }

    auto base_def = find_definition(family.base_id);
    vx::throw_if_fmt(base_def == nullptr, "block_registry: {}: invalid base_id: {}", family.name.full_string(), family.base_id);
    BlockDefinition resolved = base_def[0];

    for(const auto& rule : family.variants) {
        auto matches = true;

        for(const auto& it : rule.when) {
            auto jt = map.find(it.first);

            if(jt == map.cend() || jt->second != it.second) {
                matches = false;
                break;
            }
        }

        if(matches) {
            resolved = BlockOverridePatch::apply(base_def[0], rule.overrides);
            break;
        }
    }

    resolved.family = def->family;

    auto new_id = static_cast<block_id_type>(s_definitions.size());
    family.resolved_states.insert_or_assign(hash, new_id);
    s_definitions.push_back(std::move(resolved));

    return new_id;
}
