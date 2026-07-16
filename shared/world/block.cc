#include "shared/pch.hh"

#include "shared/world/block.hh"

std::optional<std::span<const Identifier>> BlockDefinition::resolve_texture_slot(std::string_view slot) const
{
    auto it = textures.find(std::string(slot));

    if(it == textures.cend() || it->second.empty()) {
        it = textures.find("default");
    }

    if(it == textures.cend() || it->second.empty()) {
        return std::nullopt;
    }

    return std::span<const Identifier>(it->second);
}
