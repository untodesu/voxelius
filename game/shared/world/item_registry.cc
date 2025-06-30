#include "shared/pch.hh"

#include "shared/world/item_registry.hh"

#include "core/math/crc64.hh"

#include "shared/world/voxel_registry.hh"

std::unordered_map<std::string, world::ItemInfoBuilder> world::item_registry::builders = {};
std::unordered_map<std::string, item_id> world::item_registry::names = {};
std::vector<std::shared_ptr<world::ItemInfo>> world::item_registry::items = {};

world::ItemInfoBuilder::ItemInfoBuilder(const char* name)
{
    prototype.name = name;
    prototype.texture = std::string();
    prototype.place_voxel = NULL_VOXEL_ID;
    prototype.cached_texture = nullptr;
}

world::ItemInfoBuilder& world::ItemInfoBuilder::set_texture(const char* texture)
{
    prototype.texture = texture;
    prototype.cached_texture = nullptr;
    return *this;
}

world::ItemInfoBuilder& world::ItemInfoBuilder::set_place_voxel(voxel_id place_voxel)
{
    prototype.place_voxel = place_voxel;
    return *this;
}

item_id world::ItemInfoBuilder::build(void) const
{
    const auto it = world::item_registry::names.find(prototype.name);

    if(it != world::item_registry::names.cend()) {
        spdlog::warn("item_registry: cannot build {}: name already present", prototype.name);
        return it->second;
    }

    auto new_info = std::make_shared<ItemInfo>();
    new_info->name = prototype.name;
    new_info->texture = prototype.texture;
    new_info->place_voxel = prototype.place_voxel;
    new_info->cached_texture = nullptr;

    world::item_registry::items.push_back(new_info);
    world::item_registry::names.insert_or_assign(prototype.name, static_cast<item_id>(world::item_registry::items.size()));

    return static_cast<item_id>(world::item_registry::items.size());
}

world::ItemInfoBuilder& world::item_registry::construct(const char* name)
{
    const auto it = world::item_registry::builders.find(name);

    if(it != world::item_registry::builders.cend()) {
        return it->second;
    } else {
        return world::item_registry::builders.emplace(name, ItemInfoBuilder(name)).first->second;
    }
}

world::ItemInfo* world::item_registry::find(const char* name)
{
    const auto it = world::item_registry::names.find(name);

    if(it != world::item_registry::names.cend()) {
        return world::item_registry::find(it->second);
    } else {
        return nullptr;
    }
}

world::ItemInfo* world::item_registry::find(const item_id item)
{
    if((item != NULL_ITEM_ID) && (item <= world::item_registry::items.size())) {
        return world::item_registry::items[item - 1].get();
    } else {
        return nullptr;
    }
}

void world::item_registry::purge(void)
{
    world::item_registry::builders.clear();
    world::item_registry::names.clear();
    world::item_registry::items.clear();
}

std::uint64_t world::item_registry::calcualte_checksum(void)
{
    std::uint64_t result = 0;

    for(const auto& info : world::item_registry::items) {
        result = math::crc64(info->name, result);
        result += static_cast<std::uint64_t>(info->place_voxel);
    }

    return result;
}
