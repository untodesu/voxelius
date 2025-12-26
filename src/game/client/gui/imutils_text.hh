#pragma once

namespace imutils
{
void text_nw(ImDrawList* list, std::string_view text, const ImVec2& pos, ImU32 color, ImU32 shadow, ImFont* font);
void text_nw(ImDrawList* list, std::string_view text, const ImVec2& pos, ImU32 color, ImU32 shadow, ImFont* font, float size);
void text_wr(ImDrawList* list, std::string_view text, const ImVec2& pos, ImU32 color, ImU32 shadow, ImFont* font, float wrap);
void text_wr(ImDrawList* list, std::string_view text, const ImVec2& pos, ImU32 color, ImU32 shadow, ImFont* font, float wrap, float size);
} // namespace imutils
