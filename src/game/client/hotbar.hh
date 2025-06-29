#ifndef CLIENT_HOTBAR_HH
#define CLIENT_HOTBAR_HH 1
#pragma once

#include "shared/types.hh"

// TODO: design an inventory system and an item
// registry and integrate the hotbar into that system

constexpr static unsigned int HOTBAR_SIZE = 9U;

namespace hotbar
{
extern unsigned int active_slot;
extern item_id slots[HOTBAR_SIZE];
} // namespace hotbar

namespace hotbar
{
void init(void);
void deinit(void);
void layout(void);
} // namespace hotbar

namespace hotbar
{
void next_slot(void);
void prev_slot(void);
} // namespace hotbar

#endif // CLIENT_HOTBAR_HH
