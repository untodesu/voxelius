#include "client/pch.hh"

#include "client/gui/checkbox.hh"

#include "client/language.hh"
#include "core/cvar/host.hh"

gui::CheckBox& gui::CheckBox::bind(std::string_view name)
{
    set_key(name);

    cvar::bind_var(m_value, name);

    return self();
}

bool gui::CheckBox::value(void) const
{
    return m_value.value();
}

void gui::CheckBox::set_value(bool value)
{
    m_value.set_value(value);
}

void gui::CheckBox::layout_control(void)
{
    auto current = m_value.value();
    auto label = current ? m_label_true.c_str() : m_label_false.c_str();
    auto width = ImGui::CalcItemWidth();

    if(ImGui::Button(label, ImVec2(width, 0.0f))) {
        m_value.set_value(!current);
    }
}

void gui::CheckBox::translate_control(void)
{
    m_label_false = language::resolve("settings.checkbox.false");
    m_label_false += imgui_id();

    m_label_true = language::resolve("settings.checkbox.true");
    m_label_true += imgui_id();
}
