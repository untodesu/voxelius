#include "client/pch.hh"

#include "client/gui/list_box.hh"

gui::ListBox& gui::ListBox::set_size(ImVec2 size)
{
    m_size = size;

    return self();
}

void gui::ListBox::layout(void)
{
    auto& control_id = imgui_id();

    if(ImGui::BeginChild(control_id.c_str(), m_size, ImGuiChildFlags_FrameStyle, ImGuiWindowFlags_AlwaysVerticalScrollbar)) {
        ContainerBuilder<ListBox>::layout();
    }

    ImGui::EndChild();
}
