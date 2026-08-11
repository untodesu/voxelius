#include "client/pch.hh"

#include "client/gui/toggle_button.hh"

#include "client/language.hh"

gui::ToggleButton& gui::ToggleButton::set_text(std::string_view key)
{
    m_text_key = key;

    return *this;
}

gui::ToggleButton& gui::ToggleButton::set_size(float wide, float tall)
{
    m_size.x = wide;
    m_size.y = tall;

    return *this;
}

gui::ToggleButton& gui::ToggleButton::on_click(std::function<void(void)> callback)
{
    m_callback = std::move(callback);

    return *this;
}

void gui::ToggleButton::set_pressed(bool pressed)
{
    m_pressed = pressed;
}

bool gui::ToggleButton::pressed(void) const
{
    return m_pressed;
}

void gui::ToggleButton::layout(void)
{
    if(m_pressed) {
        ImVec4 color(0.500f, 0.500f, 0.500f, 0.500f);
        ImGui::PushStyleColor(ImGuiCol_Button, color);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, color);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, color);
    }
    else {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.000f, 0.000f, 0.000f, 0.000f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.500f, 0.500f, 0.500f, 0.125f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.500f, 0.500f, 0.500f, 0.500f));
    }

    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);

    if(ImGui::Button(m_text.c_str(), m_size)) {
        if(m_callback) {
            m_callback();
        }
    }

    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);
}

void gui::ToggleButton::translate(void)
{
    m_text = language::resolve(m_text_key);
    m_text += imgui_id();
}
