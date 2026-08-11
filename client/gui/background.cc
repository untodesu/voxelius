#include "client/pch.hh"

#include "client/gui/background.hh"

#include "client/globals.hh"

gui::Background& gui::Background::set_left(const ImVec4& color)
{
    m_left = color;

    return self();
}

gui::Background& gui::Background::set_right(const ImVec4& color)
{
    m_right = color;

    return self();
}

gui::Background& gui::Background::set_texture(ImTextureID texture)
{
    m_texture = texture;

    return self();
}

void gui::Background::layout(void)
{
    auto draw_list = ImGui::GetWindowDrawList();
    auto window_pos = ImGui::GetWindowPos();
    auto window_size = ImGui::GetWindowSize();

    auto left = ImGui::GetColorU32(m_left);
    auto right = ImGui::GetColorU32(m_right);

    ImVec2 window_end;
    window_end.x = window_pos.x + window_size.x;
    window_end.y = window_pos.y + window_size.y;

    if(m_texture == 0 || globals::registry.valid(globals::player)) {
        draw_list->AddRectFilledMultiColor(window_pos, window_end, left, right, right, left);
    }
    else {
        draw_list->AddImage(m_texture, window_pos, window_end);
    }
}
