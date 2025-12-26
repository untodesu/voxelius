// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: hotbar.hh
// Description: Hotbar rendering

#ifndef CLIENT_GUI_HOTBAR_HH
#define CLIENT_GUI_HOTBAR_HH
#pragma once

// TODO: design an inventory system and an item
// registry and integrate the hotbar into that system

class Item;

constexpr static unsigned int HOTBAR_SIZE = 9U;

namespace hotbar
{
extern unsigned int active_slot;
extern std::array<const Item*, HOTBAR_SIZE> slots;
} // namespace hotbar

namespace hotbar
{
void init(void);
void shutdown(void);
void layout(void);
} // namespace hotbar

namespace hotbar
{
void next_slot(void);
void prev_slot(void);
} // namespace hotbar

#endif
