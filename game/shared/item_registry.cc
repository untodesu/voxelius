#include "shared/pch.hh"
#include "shared/item_registry.hh"

#include "shared/voxel_registry.hh"

std::unordered_map<std::string, ItemInfoBuilder> item_registry::builders = {};
std::unordered_map<std::string, item_id> item_registry::names = {};
std::vector<std::shared_ptr<ItemInfo>> item_registry::items = {};

ItemInfoBuilder::ItemInfoBuilder(const char *name)
{
    prototype.name = name;
    prototype.texture = std::string();
    prototype.place_voxel = NULL_VOXEL_ID;
    prototype.cached_texture = nullptr;
}

ItemInfoBuilder &ItemInfoBuilder::set_texture(const char *texture)
{
    prototype.texture = texture;
    prototype.cached_texture = nullptr;
    return *this;
}

ItemInfoBuilder &ItemInfoBuilder::set_place_voxel(voxel_id place_voxel)
{
    prototype.place_voxel = place_voxel;
    return *this;
}

item_id ItemInfoBuilder::build(void) const
{
    const auto it = item_registry::names.find(prototype.name);

    if(it != item_registry::names.cend()) {
        spdlog::warn("item_registry: cannot build {}: name already present", prototype.name);
        return it->second;
    }

    auto new_info = std::make_shared<ItemInfo>();
    new_info->name = prototype.name;
    new_info->texture = prototype.texture;
    new_info->place_voxel = prototype.place_voxel;
    new_info->cached_texture = nullptr;

    item_registry::items.push_back(new_info);
    item_registry::names.insert_or_assign(prototype.name, static_cast<item_id>(item_registry::items.size()));

    return static_cast<item_id>(item_registry::items.size());
}

ItemInfoBuilder &item_registry::construct(const char *name)
{
    const auto it = item_registry::builders.find(name);
    if(it != item_registry::builders.cend())
        return it->second;
    return item_registry::builders.emplace(name, ItemInfoBuilder(name)).first->second;
}

ItemInfo *item_registry::find(const char *name)
{
    const auto it = item_registry::names.find(name);
    if(it != item_registry::names.cend())
        return item_registry::find(it->second);
    return nullptr;
}

ItemInfo *item_registry::find(const item_id item)
{
    if((item != NULL_ITEM_ID) && (item <= item_registry::items.size()))
        return item_registry::items[item - 1].get();
    return nullptr;
}

void item_registry::purge(void)
{
    item_registry::builders.clear();
    item_registry::names.clear();
    item_registry::items.clear();
}
