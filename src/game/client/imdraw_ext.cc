#include "client/pch.hh"

#include "client/imdraw_ext.hh"

#include "client/globals.hh"

void imdraw_ext::text_shadow(
    const std::string& text, const ImVec2& position, ImU32 text_color, ImU32 shadow_color, ImFont* font, ImDrawList* draw_list)
{
    const auto shadow_position = ImVec2(position.x + 0.5f * globals::gui_scale, position.y + 0.5f * globals::gui_scale);
    draw_list->AddText(font, font->FontSize, shadow_position, shadow_color, text.c_str(), text.c_str() + text.size());
    draw_list->AddText(font, font->FontSize, position, text_color, text.c_str(), text.c_str() + text.size());
}
