#include "shared/pch.hh"

#include "shared/world/block.hh"

std::optional<std::span<const Identifier>> BlockDefinition::resolve_albedo_slot(std::string_view slot) const
{
    auto it = albedo.find(std::string(slot));

    if(it == albedo.cend() || it->second.empty()) {
        it = albedo.find("default");
    }

    if(it == albedo.cend() || it->second.empty()) {
        return std::nullopt;
    }

    return std::span<const Identifier>(it->second);
}

std::optional<Identifier> BlockDefinition::resolve_mask_slot(std::string_view slot) const
{
    auto it = masks.find(std::string(slot));

    if(it == masks.cend()) {
        return std::nullopt;
    }

    return it->second;
}
