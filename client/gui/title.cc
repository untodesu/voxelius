#include "client/pch.hh"

#include "client/gui/title.hh"

#include "client/fonts.hh"
#include "client/language.hh"

gui::Title& gui::Title::set_text(std::string_view key)
{
    m_key = key;

    return self();
}

void gui::Title::layout(void)
{
    ImGui::PushFont(fonts::unscii8, 24.0f);
    ImGui::TextUnformatted(m_text.c_str());
    ImGui::PopFont();
}

void gui::Title::translate(void)
{
    m_text = language::resolve(m_key);
}
