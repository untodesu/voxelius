#ifndef SHARED_GAME_ITEMS
#define SHARED_GAME_ITEMS 1
#pragma once

#include "shared/types.hh"

namespace game_items
{
extern item_id stone;
extern item_id cobblestone;
extern item_id dirt;
extern item_id grass;
extern item_id oak_leaves;
extern item_id oak_planks;
extern item_id oak_log;
extern item_id glass;
extern item_id slime;
extern item_id mud;
} // namespace game_items

namespace game_items
{
void populate(void);
} // namespace game_items

#endif // SHARED_GAME_ITEMS
