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

void gui::CheckBox::layout_control(void)
{
    auto current = m_value.value();

    auto control_width = ImGui::CalcItemWidth();
    auto box_size = ImGui::GetFrameHeight();

    auto cursor_x = ImGui::GetCursorPosX();
    ImGui::SetCursorPosX(cursor_x + control_width - box_size);

    auto& id = imgui_id();

    if(ImGui::Checkbox(id.c_str(), &current)) {
        m_value.set_value(current);
    }
}

void gui::CheckBox::translate_control(void)
{
    // empty
}
