#include "client/pch.hh"

#include "client/gui/imdraw_ext.hh"

#include "client/globals.hh"

void gui::imdraw_ext::text_shadow(const std::string& text, const ImVec2& position, ImU32 text_color, ImU32 shadow_color, ImFont* font,
    ImDrawList* draw_list)
{
    imdraw_ext::text_shadow(text, position, text_color, shadow_color, font, draw_list, font->LegacySize);
}

void gui::imdraw_ext::text_shadow(const std::string& text, const ImVec2& position, ImU32 text_color, ImU32 shadow_color, ImFont* font,
    ImDrawList* draw_list, float font_size)
{
    const auto shadow_position = ImVec2(position.x + 0.5f * globals::gui_scale, position.y + 0.5f * globals::gui_scale);
    draw_list->AddText(font, globals::gui_scale * font_size, shadow_position, shadow_color, text.c_str(), text.c_str() + text.size());
    draw_list->AddText(font, globals::gui_scale * font_size, position, text_color, text.c_str(), text.c_str() + text.size());
}

void gui::imdraw_ext::text_shadow_w(const std::string& text, const ImVec2& position, ImU32 text_color, ImU32 shadow_color, ImFont* font,
    ImDrawList* draw_list, float wrap_width)
{
    imdraw_ext::text_shadow_w(text, position, text_color, shadow_color, font, draw_list, font->LegacySize, wrap_width);
}

void gui::imdraw_ext::text_shadow_w(const std::string& text, const ImVec2& position, ImU32 text_color, ImU32 shadow_color, ImFont* font,
    ImDrawList* draw_list, float font_size, float wrap_width)
{
    const auto shadow_position = ImVec2(position.x + 0.5f * globals::gui_scale, position.y + 0.5f * globals::gui_scale);
    draw_list->AddText(font, globals::gui_scale * font_size, shadow_position, shadow_color, text.c_str(), text.c_str() + text.size(),
        wrap_width);
    draw_list->AddText(font, globals::gui_scale * font_size, position, text_color, text.c_str(), text.c_str() + text.size(), wrap_width);
}
