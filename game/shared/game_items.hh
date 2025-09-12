#pragma once

namespace world
{
class Item;
} // namespace world

namespace game_items
{
extern const world::Item* stone;
extern const world::Item* cobblestone;
extern const world::Item* dirt;
extern const world::Item* grass;
extern const world::Item* oak_leaves;
extern const world::Item* oak_planks;
extern const world::Item* oak_log;
extern const world::Item* glass;
extern const world::Item* slime;
} // namespace game_items

namespace game_items
{
void populate(void);
} // namespace game_items
