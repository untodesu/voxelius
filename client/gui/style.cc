#include "client/pch.hh"

#include "client/gui/style.hh"

void gui::detail::apply_style(void)
{
    ImGui::StyleColorsDark();

    auto& style = ImGui::GetStyle();
    auto& colors = style.Colors;

    style.Alpha = 1.0f;
    style.DisabledAlpha = 0.6f;

    style.WindowMinSize = ImVec2(32.0f, 32.0f);
    style.IndentSpacing = 21.0f;
    style.ColumnsMinSpacing = 6.0f;
    style.ScrollbarSize = 14.0f;
    style.GrabMinSize = 10.0f;

    style.WindowPadding = ImVec2(8.0f, 8.0f);
    style.FramePadding = ImVec2(4.0f, 3.0f);
    style.ItemSpacing = ImVec2(8.0f, 4.0f);
    style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
    style.CellPadding = ImVec2(4.0f, 2.0f);

    style.WindowRounding = 0.0f;
    style.ChildRounding = 0.0f;
    style.PopupRounding = 0.0f;
    style.FrameRounding = 0.0f;
    style.ScrollbarRounding = 9.0f;
    style.GrabRounding = 0.0f;
    style.TabRounding = 0.0f;

    style.WindowBorderSize = 1.0f;
    style.ChildBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;
    style.TabBorderSize = 1.0f;

    style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
    style.WindowMenuButtonPosition = ImGuiDir_Left;
    style.ColorButtonPosition = ImGuiDir_Right;
    style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
    style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

    colors[ImGuiCol_Text] = ImVec4(1.000f, 1.000f, 1.000f, 1.000f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.729f, 0.749f, 0.737f, 1.000f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.086f, 0.086f, 0.086f, 0.940f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.000f, 0.000f, 0.000f, 0.000f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.078f, 0.078f, 0.078f, 0.940f);
    colors[ImGuiCol_Border] = ImVec4(0.137f, 0.137f, 0.137f, 1.000f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.137f, 0.137f, 0.137f, 1.000f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.000f, 0.000f, 0.000f, 0.540f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.176f, 0.176f, 0.176f, 0.400f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.216f, 0.216f, 0.216f, 0.670f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.137f, 0.137f, 0.137f, 1.000f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.137f, 0.137f, 0.137f, 1.000f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.137f, 0.137f, 0.137f, 0.670f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.216f, 0.216f, 0.216f, 1.000f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.020f, 0.020f, 0.020f, 0.530f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.310f, 0.310f, 0.310f, 1.000f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.408f, 0.408f, 0.408f, 1.000f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.510f, 0.510f, 0.510f, 1.000f);
    colors[ImGuiCol_CheckMark] = ImVec4(1.000f, 0.000f, 0.000f, 1.000f);
    colors[ImGuiCol_SliderGrab] = ImVec4(1.000f, 0.000f, 0.000f, 1.000f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(1.000f, 0.380f, 0.380f, 1.000f);
    colors[ImGuiCol_Button] = ImVec4(0.000f, 0.000f, 0.000f, 0.541f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.176f, 0.176f, 0.176f, 0.400f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.216f, 0.216f, 0.216f, 0.671f);
    colors[ImGuiCol_Header] = ImVec4(0.216f, 0.216f, 0.216f, 1.000f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.271f, 0.271f, 0.271f, 1.000f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.353f, 0.353f, 0.353f, 1.000f);
    colors[ImGuiCol_Separator] = ImVec4(1.000f, 1.000f, 1.000f, 1.000f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(1.000f, 0.000f, 0.000f, 1.000f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(1.000f, 0.329f, 0.329f, 1.000f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(1.000f, 0.000f, 0.000f, 1.000f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.000f, 0.486f, 0.486f, 1.000f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(1.000f, 0.486f, 0.486f, 1.000f);
    colors[ImGuiCol_Tab] = ImVec4(0.220f, 0.220f, 0.220f, 1.000f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.290f, 0.290f, 0.290f, 1.000f);
    colors[ImGuiCol_TabActive] = ImVec4(0.176f, 0.176f, 0.176f, 1.000f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.149f, 0.067f, 0.067f, 0.970f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.404f, 0.153f, 0.153f, 1.000f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.608f, 0.608f, 0.608f, 1.000f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.000f, 0.000f, 0.000f, 1.000f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.898f, 0.000f, 0.000f, 1.000f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.365f, 0.000f, 0.000f, 1.000f);
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.302f, 0.302f, 0.302f, 1.000f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.137f, 0.137f, 0.137f, 1.000f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.137f, 0.137f, 0.137f, 1.000f);
    colors[ImGuiCol_TableRowBg] = ImVec4(0.000f, 0.000f, 0.000f, 0.000f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.000f, 1.000f, 1.000f, 0.060f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.263f, 0.635f, 0.878f, 0.438f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(0.467f, 0.184f, 0.184f, 0.966f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.408f, 0.408f, 0.408f, 1.000f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.000f, 1.000f, 1.000f, 0.700f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.800f, 0.800f, 0.800f, 0.200f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.800f, 0.800f, 0.800f, 0.350f);
}
