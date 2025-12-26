// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: status_lines.hh
// Description: Display lines of text in-game

#ifndef CLIENT_GUI_STATUS_LINES_HH
#define CLIENT_GUI_STATUS_LINES_HH
#pragma once

constexpr static unsigned int STATUS_DEBUG = 0x0000;  // generic debug line
constexpr static unsigned int STATUS_HOTBAR = 0x0001; // hotbar item line
constexpr static unsigned int STATUS_COUNT = 0x0002;

namespace status_lines
{
void init(void);
void init_late(void);
void layout(void);
} // namespace status_lines

namespace status_lines
{
void set(unsigned int line, std::string_view text, const ImVec4& color, float fadeout);
void unset(unsigned int line);
} // namespace status_lines

#endif
