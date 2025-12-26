// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: chat.hh
// Description: Client chat handling

#ifndef CLIENT_GUI_CHAT_HH
#define CLIENT_GUI_CHAT_HH
#pragma once

namespace client_chat
{
void init(void);
void init_late(void);
void shutdown(void);
void update(void);
void layout(void);
} // namespace client_chat

namespace client_chat
{
void clear(void);
void refresh_timings(void);
void print(const std::string& string);
} // namespace client_chat

#endif
