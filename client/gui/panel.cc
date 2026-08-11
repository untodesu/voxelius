#include "client/pch.hh"

#include "client/gui/panel.hh"

#include "client/globals.hh"

gui::Panel& gui::Panel::set_margin(ImVec2 margin)
{
    m_margin = std::move(margin);

    return self();
}

gui::Panel& gui::Panel::enable_background(void)
{
    m_background = true;

    return self();
}

void gui::Panel::layout(void)
{
    ImVec2 margin {};
    margin.x = m_margin.x * static_cast<float>(globals::gui_scale);
    margin.y = m_margin.y * static_cast<float>(globals::gui_scale);

    auto avail = ImGui::GetContentRegionAvail();

    ImVec2 size {};
    size.x = avail.x - 2.0f * margin.x;
    size.y = avail.y - 2.0f * margin.y;

    auto cursor_pos = ImGui::GetCursorPos();
    cursor_pos.x += margin.x;
    cursor_pos.y += margin.y;

    ImGui::SetCursorPos(cursor_pos);

    auto& window_id = imgui_id();
    auto window_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBackground;

    if(m_background) {
        window_flags &= ~ImGuiWindowFlags_NoBackground;
    }

    if(ImGui::BeginChild(window_id.c_str(), size, ImGuiChildFlags_None, window_flags)) {
        ContainerBuilder<Panel>::layout();
    }

    ImGui::EndChild();
}
