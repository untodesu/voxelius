#ifndef CLIENT_IMDRAW_EXT_HH
#define CLIENT_IMDRAW_EXT_HH 1
#pragma once

namespace imdraw_ext
{
void text_shadow(const std::string &text, const ImVec2 &position, ImU32 text_color, ImU32 shadow_color, ImFont *font, ImDrawList *draw_list);
} // namespace imdraw_ext

#endif /* CLIENT_IMDRAW_EXT_HH */
