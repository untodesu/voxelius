// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: message_box.hh
// Description: A single message screen

#ifndef CLIENT_GUI_MESSAGE_BOX_HH
#define CLIENT_GUI_MESSAGE_BOX_HH
#pragma once

using message_box_action = void (*)(void);

namespace message_box
{
void init(void);
void layout(void);
void reset(void);
} // namespace message_box

namespace message_box
{
void set_title(std::string_view title);
void set_subtitle(std::string_view subtitle);
void add_button(std::string_view text, const message_box_action& action);
} // namespace message_box

#endif
