// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: game_items.hh
// Description: All the items in the game

#ifndef SHARED_GAME_ITEMS_HH
#define SHARED_GAME_ITEMS_HH
#pragma once

class Item;

namespace game_items
{
extern const Item* stone;
extern const Item* cobblestone;
extern const Item* dirt;
extern const Item* grass;
extern const Item* oak_leaves;
extern const Item* oak_planks;
extern const Item* oak_log;
extern const Item* glass;
extern const Item* slime;
} // namespace game_items

namespace game_items
{
void populate(void);
} // namespace game_items

#endif
