#pragma once

namespace gui::imdraw_ext
{
void text_shadow(const std::string& text, const ImVec2& position, ImU32 text_color, ImU32 shadow_color, ImFont* font,
    ImDrawList* draw_list);
void text_shadow(const std::string& text, const ImVec2& position, ImU32 text_color, ImU32 shadow_color, ImFont* font, ImDrawList* draw_list,
    float font_size);
} // namespace gui::imdraw_ext

namespace gui::imdraw_ext
{
void text_shadow_w(const std::string& text, const ImVec2& position, ImU32 text_color, ImU32 shadow_color, ImFont* font,
    ImDrawList* draw_list, float wrap_width);
void text_shadow_w(const std::string& text, const ImVec2& position, ImU32 text_color, ImU32 shadow_color, ImFont* font,
    ImDrawList* draw_list, float font_size, float wrap_width);
} // namespace gui::imdraw_ext
