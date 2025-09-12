#include "shared/pch.hh"

#include "shared/world/item_registry.hh"

#include "core/math/crc64.hh"

#include "shared/world/voxel_registry.hh"

static std::uint64_t registry_checksum = 0U;
std::unordered_map<std::string, item_id> world::item_registry::names = {};
std::vector<std::unique_ptr<world::Item>> world::item_registry::items = {};

static void recalculate_checksum(void)
{
    registry_checksum = 0U;

    for(const auto& item : world::item_registry::items) {
        registry_checksum = item->get_checksum(registry_checksum);
    }
}

world::Item* world::item_registry::register_item(const ItemBuilder& builder)
{
    assert(builder.get_name().size());
    assert(nullptr == find(builder.get_name()));

    const auto id = static_cast<item_id>(1 + items.size());

    std::unique_ptr<Item> item(builder.build(id));
    names.emplace(std::string(builder.get_name()), id);
    items.push_back(std::move(item));

    recalculate_checksum();

    return items.back().get();
}

world::Item* world::item_registry::find(std::string_view name)
{
    const auto it = names.find(std::string(name));

    if(it == names.end()) {
        return nullptr;
    }

    return items[it->second - 1].get();
}

world::Item* world::item_registry::find(const item_id item)
{
    if(item == NULL_ITEM_ID || item > items.size()) {
        return nullptr;
    }

    return items[item - 1].get();
}

void world::item_registry::purge(void)
{
    registry_checksum = 0U;
    items.clear();
    names.clear();
}

std::uint64_t world::item_registry::get_checksum(void)
{
    return registry_checksum;
}
