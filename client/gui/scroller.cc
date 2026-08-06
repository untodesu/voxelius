#include "client/pch.hh"

#include "client/gui/scroller.hh"

#include "client/globals.hh"

gui::Scroller& gui::Scroller::set_margin(ImVec2 margin)
{
    m_margin = std::move(margin);

    return self();
}

gui::Scroller& gui::Scroller::set_scroll_speed(float speed)
{
    m_scroll_speed = speed;

    return self();
}

void gui::Scroller::layout(void)
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

    if(ImGui::BeginChild(imgui_id().c_str(), size)) {
        if(ImGui::IsWindowHovered() || ImGui::IsWindowFocused()) {
            auto speed = m_scroll_speed * static_cast<float>(globals::gui_scale);

            if(ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
                ImGui::SetScrollY(ImGui::GetScrollY() + speed);
            }

            if(ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
                ImGui::SetScrollY(ImGui::GetScrollY() - speed);
            }
        }

        ContainerBuilder<Scroller>::layout();
    }

    ImGui::EndChild();
}
