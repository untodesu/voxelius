// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: imutils_text.hh
// Description: Text with shadow

#ifndef CLIENT_GUI_IMUTILS_TEXT_HH
#define CLIENT_GUI_IMUTILS_TEXT_HH
#pragma once

namespace imutils
{
void text_nw(ImDrawList* list, std::string_view text, const ImVec2& pos, ImU32 color, ImU32 shadow, ImFont* font);
void text_nw(ImDrawList* list, std::string_view text, const ImVec2& pos, ImU32 color, ImU32 shadow, ImFont* font, float size);
void text_wr(ImDrawList* list, std::string_view text, const ImVec2& pos, ImU32 color, ImU32 shadow, ImFont* font, float wrap);
void text_wr(ImDrawList* list, std::string_view text, const ImVec2& pos, ImU32 color, ImU32 shadow, ImFont* font, float wrap, float size);
} // namespace imutils

#endif
