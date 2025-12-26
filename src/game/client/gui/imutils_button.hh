// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: imutils_button.hh
// Description: Extended button layout

#ifndef CLIENT_GUI_IMUTILS_BUTTON_HH
#define CLIENT_GUI_IMUTILS_BUTTON_HH
#pragma once

namespace imutils
{
void button(const char* title, const ImVec2& size, void (*callback)(void));
void button(const char* title, void (*callback)(void));
} // namespace imutils

namespace imutils
{
bool selectable_button(const char* label, const ImVec2& size, bool value);
bool toggle_button(const char* label, const ImVec2& size, bool& value);
bool toggle_button(const char* label, bool& value);
} // namespace imutils

#endif
