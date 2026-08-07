#include "client/pch.hh"

#include "client/gui/checkbox.hh"

#include "core/config/map.hh"

#include "client/language.hh"

gui::CheckBox& gui::CheckBox::bind(config::Map& config, std::string_view key)
{
    set_key(key);

    m_value.bind(config, key);

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

bool gui::CheckBox::dirty(void) const
{
    return m_value.dirty();
}

void gui::CheckBox::layout_control(void)
{
    auto current = m_value.value();
    auto& control_id = imgui_id();

    if(ImGui::Checkbox(control_id.c_str(), &current)) {
        m_value.set_value(current);
    }
}

void gui::CheckBox::translate_control(void)
{
    // empty
}
