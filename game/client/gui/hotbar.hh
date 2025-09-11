#pragma once

#include "shared/types.hh"

// TODO: design an inventory system and an item
// registry and integrate the hotbar into that system

constexpr static unsigned int HOTBAR_SIZE = 9U;

namespace gui::hotbar
{
extern unsigned int active_slot;
extern item_id slots[HOTBAR_SIZE];
} // namespace gui::hotbar

namespace gui::hotbar
{
void init(void);
void shutdown(void);
void layout(void);
} // namespace gui::hotbar

namespace gui::hotbar
{
void next_slot(void);
void prev_slot(void);
} // namespace gui::hotbar
