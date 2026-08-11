#include "client/pch.hh"

#include "client/gui/label.hh"

#include "client/language.hh"

gui::Label& gui::Label::set_text(std::string_view key)
{
    m_key = key;

    return self();
}

void gui::Label::layout(void)
{
    ImGui::TextUnformatted(m_text.c_str());
}

void gui::Label::translate(void)
{
    m_text = language::resolve(m_key);
}
