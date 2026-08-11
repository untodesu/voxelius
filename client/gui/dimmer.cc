#include "client/pch.hh"

#include "client/gui/dimmer.hh"

gui::Dimmer& gui::Dimmer::set_color(const ImVec4& color)
{
    m_top_left = color;
    m_top_right = color;
    m_bottom_left = color;
    m_bottom_right = color;

    return self();
}

gui::Dimmer& gui::Dimmer::set_top(const ImVec4& color)
{
    m_top_left = color;
    m_top_right = color;

    return self();
}

gui::Dimmer& gui::Dimmer::set_bottom(const ImVec4& color)
{
    m_bottom_left = color;
    m_bottom_right = color;

    return self();
}

gui::Dimmer& gui::Dimmer::set_top_left(const ImVec4& color)
{
    m_top_left = color;

    return self();
}

gui::Dimmer& gui::Dimmer::set_top_right(const ImVec4& color)
{
    m_top_right = color;

    return self();
}

gui::Dimmer& gui::Dimmer::set_bottom_left(const ImVec4& color)
{
    m_bottom_left = color;

    return self();
}

gui::Dimmer& gui::Dimmer::set_bottom_right(const ImVec4& color)
{
    m_bottom_right = color;

    return self();
}

void gui::Dimmer::layout(void)
{
    auto draw_list = ImGui::GetWindowDrawList();
    auto window_pos = ImGui::GetWindowPos();
    auto window_size = ImGui::GetWindowSize();

    auto top_left = ImGui::GetColorU32(m_top_left);
    auto top_right = ImGui::GetColorU32(m_top_right);
    auto bottom_left = ImGui::GetColorU32(m_bottom_left);
    auto bottom_right = ImGui::GetColorU32(m_bottom_right);

    ImVec2 window_end;
    window_end.x = window_pos.x + window_size.x;
    window_end.y = window_pos.y + window_size.y;

    draw_list->AddRectFilledMultiColor(window_pos, window_end, top_left, top_right, bottom_right, bottom_left);
}
