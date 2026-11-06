#include "client/pch.hh"

#include "client/gui/font_scope.hh"

gui::FontScope& gui::FontScope::set_font(ImFont* font, float size)
{
    m_font = font;
    m_size = size;

    return self();
}

void gui::FontScope::layout(void)
{
    ImGui::PushFont(m_font, m_size);
    ContainerBuilder<FontScope>::layout();
    ImGui::PopFont();
}
