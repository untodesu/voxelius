// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: item_registry.hh
// Description: Registry for all the items in the game

#ifndef SHARED_WORLD_ITEM_REGISTRY_HH
#define SHARED_WORLD_ITEM_REGISTRY_HH
#pragma once

#include "shared/world/item.hh"

namespace item_registry
{
extern std::unordered_map<std::string, item_id> names;
extern std::vector<std::unique_ptr<Item>> items;
} // namespace item_registry

namespace item_registry
{
Item* register_item(const ItemBuilder& builder);
Item* find(std::string_view name);
Item* find(const item_id item);
} // namespace item_registry

namespace item_registry
{
void purge(void);
} // namespace item_registry

namespace item_registry
{
std::uint64_t get_checksum(void);
} // namespace item_registry

#endif
