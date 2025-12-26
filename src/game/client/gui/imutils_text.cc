// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: imutils_text.cc
// Description: Text with shadow

#include "client/pch.hh"

#include "client/gui/imutils_text.hh"

#include "client/globals.hh"

void imutils::text_nw(ImDrawList* list, std::string_view text, const ImVec2& pos, ImU32 color, ImU32 shadow, ImFont* font)
{
    assert(list);
    assert(font);

    imutils::text_nw(list, text, pos, color, shadow, font, font->LegacySize);
}

void imutils::text_nw(ImDrawList* list, std::string_view text, const ImVec2& pos, ImU32 color, ImU32 shadow, ImFont* font, float size)
{
    assert(list);
    assert(font);

    auto shift = static_cast<float>(globals::gui_scale);
    auto shadow_pos = ImVec2(pos.x + shift, pos.y + shift);

    list->AddText(font, size, shadow_pos, shadow, text.data(), text.data() + text.size());
    list->AddText(font, size, pos, color, text.data(), text.data() + text.size());
}

void imutils::text_wr(ImDrawList* list, std::string_view text, const ImVec2& pos, ImU32 color, ImU32 shadow, ImFont* font, float wrap)
{
    assert(list);
    assert(font);

    imutils::text_wr(list, text, pos, color, shadow, font, wrap, font->LegacySize);
}

void imutils::text_wr(ImDrawList* list, std::string_view text, const ImVec2& pos, ImU32 color, ImU32 shadow, ImFont* font, float wrap,
    float size)
{
    assert(list);
    assert(font);

    auto shift = static_cast<float>(globals::gui_scale);
    auto shadow_pos = ImVec2(pos.x + shift, pos.y + shift);

    list->AddText(font, size, shadow_pos, shadow, text.data(), text.data() + text.size(), wrap);
    list->AddText(font, size, pos, color, text.data(), text.data() + text.size(), wrap);
}
