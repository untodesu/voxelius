#pragma once

#include "shared/world/item.hh"

namespace world::item_registry
{
extern std::unordered_map<std::string, item_id> names;
extern std::vector<std::unique_ptr<Item>> items;
} // namespace world::item_registry

namespace world::item_registry
{
Item* register_item(const ItemBuilder& builder);
Item* find(std::string_view name);
Item* find(const item_id item);
} // namespace world::item_registry

namespace world::item_registry
{
void purge(void);
} // namespace world::item_registry

namespace world::item_registry
{
std::uint64_t get_checksum(void);
} // namespace world::item_registry
