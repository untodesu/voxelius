#include "shared/pch.hh"

#include "shared/block_registry.hh"

#include "core/utils/crc64.hh"

std::span<const BlockDefinition> block_registry::all_definitions(void) noexcept
{
}

std::span<const BlockFamily> block_registry::all_families(void) noexcept
{
}

void block_registry::commit(ModContext& ctx) noexcept
{
}

void block_registry::purge(void) noexcept
{
}

block_id_type block_registry::find(const Identifier& id) noexcept
{
}

const BlockDefinition* block_registry::find_definition(block_id_type id) noexcept
{
}

const BlockDefinition* block_registry::find_definition(const Identifier& id) noexcept
{
}

const BlockFamily* block_registry::find_family(block_id_type id) noexcept
{
}

const BlockFamily* block_registry::find_family(const Identifier& id) noexcept
{
}

bool block_registry::has_tag_all(block_id_type id, block_tag_bit tag_bits) noexcept
{
}

bool block_registry::has_tag_any(block_id_type id, block_tag_bit tag_bits) noexcept
{
}

block_id_type block_registry::resolve_variant(block_id_type curr_id,
    const std::unordered_map<blockstate_key_type, blockstate_val_type>& map) noexcept
{
}
