#include "client/pch.hh"

#include "client/gui/screen.hh"

gui::Screen& gui::Screen::set_start(float xrel, float yrel)
{
    m_start.x = xrel;
    m_start.y = yrel;

    return self();
}

gui::Screen& gui::Screen::set_size(float xrel, float yrel)
{
    m_size.x = xrel;
    m_size.y = yrel;

    return self();
}

gui::Screen& gui::Screen::set_flags(ImGuiWindowFlags flags)
{
    m_flags = flags;

    return self();
}

void gui::Screen::layout(void)
{
    auto viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos({ viewport->Size.x * m_start.x, viewport->Size.y * m_start.y });
    ImGui::SetNextWindowSize({ viewport->Size.x * m_size.x, viewport->Size.y * m_size.y });

    auto& window_id = imgui_id();

    if(ImGui::Begin(window_id.c_str(), nullptr, m_flags)) {
        ContainerBuilder<Screen>::layout();
    }

    ImGui::End();
}
