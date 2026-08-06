#include "client/pch.hh"

#include "client/gui/menu.hh"

#include "client/fonts.hh"
#include "client/globals.hh"
#include "client/language.hh"

static bool condition_met(unsigned condition)
{
    if(globals::registry.valid(globals::player)) {
        return static_cast<bool>(condition & gui::Menu::ALLOW_INGAME);
    }
    else {
        return static_cast<bool>(condition & gui::Menu::ALLOW_OFFLINE);
    }
}

gui::Menu& gui::Menu::set_margin(ImVec2 margin)
{
    m_margin = std::move(margin);

    return self();
}

gui::Menu& gui::Menu::set_control(ImVec2 control)
{
    m_control = std::move(control);

    return self();
}

gui::Menu& gui::Menu::add_button(std::string_view label, std::function<void(void)> callback, unsigned condition)
{
    Button item {};
    item.condition = condition;
    item.label_key = label;
    item.callback = std::move(callback);

    m_items.emplace_back(std::move(item));

    return self();
}

gui::Menu& gui::Menu::add_spacer(float height, unsigned condition)
{
    Spacer item {};
    item.condition = condition;
    item.height = height;

    m_items.emplace_back(std::move(item));

    return self();
}

void gui::Menu::layout(void)
{
    ImVec2 margin {};
    margin.x = m_margin.x * static_cast<float>(globals::gui_scale);
    margin.y = m_margin.y * static_cast<float>(globals::gui_scale);

    ImVec2 control {};
    control.x = m_control.x * static_cast<float>(globals::gui_scale);
    control.y = m_control.y * static_cast<float>(globals::gui_scale);

    auto window_size = ImGui::GetWindowSize();

    ImGui::PushFont(fonts::unscii16, 16.0f);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.000f, 0.000f, 0.000f, 0.000f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.500f, 0.500f, 0.500f, 0.125f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.500f, 0.500f, 0.500f, 0.500f));
    ImGui::PushStyleVarX(ImGuiStyleVar_FramePadding, 16.0f * static_cast<float>(globals::gui_scale));
    ImGui::PushStyleVarX(ImGuiStyleVar_ButtonTextAlign, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);

    ImVec2 cursor_pos {};
    cursor_pos.x = margin.x;
    cursor_pos.y = window_size.y - margin.y;
    cursor_pos.y -= control.y * 1.5f;

    for(const auto& item : m_items) {
        if(auto button = std::get_if<Button>(&item)) {
            if(condition_met(button->condition)) {
                ImGui::SetCursorPos(cursor_pos);

                if(ImGui::Button(button->label.c_str(), control)) {
                    if(button->callback) {
                        button->callback();
                    }
                }

                cursor_pos.y -= control.y;
            }
        }
        else if(auto spacer = std::get_if<Spacer>(&item)) {
            if(condition_met(spacer->condition)) {
                cursor_pos.y -= spacer->height * static_cast<float>(globals::gui_scale);
            }
        }
    }

    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(3);
    ImGui::PopFont();
}

void gui::Menu::translate(void)
{
    for(std::size_t i = 0; i < m_items.size(); ++i) {
        if(auto button = std::get_if<Button>(&m_items[i])) {
            button->label = language::resolve(button->label_key);
            button->label += std::format("{}[{}]", imgui_id(), i);
        }
    }
}
