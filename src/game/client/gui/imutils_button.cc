#include "client/pch.hh"

#include "client/gui/imutils_button.hh"

void imutils::button(const char* title, const ImVec2& size, void (*callback)(void))
{
    assert(title);
    assert(callback);

    if(ImGui::Button(title, size)) {
        callback();
    }
}

void imutils::button(const char* title, void (*callback)(void))
{
    assert(title);
    assert(callback);

    if(ImGui::Button(title)) {
        callback();
    }
}

bool imutils::selectable_button(const char* label, const ImVec2& size, bool value)
{
    assert(label);

    auto value_changed = false;

    if(value) {
        ImVec4 button_color(0.750f, 0.750f, 0.750f, 1.000f);
        ImGui::PushStyleColor(ImGuiCol_Button, button_color);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, button_color);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, button_color);
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_WindowBg));
    }
    else {
        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_WindowBg));
    }

    if(ImGui::Button(label, size)) {
        value_changed = true;
    }

    ImGui::PopStyleColor(value ? 4 : 1);

    return value_changed;
}

bool imutils::toggle_button(const char* label, const ImVec2& size, bool& value)
{
    assert(label);

    auto use_custom_color = !value;
    auto value_changed = false;

    if(use_custom_color) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_WindowBg));
    }

    if(ImGui::Button(label, size)) {
        value_changed = true;
        value = !value;
    }

    if(use_custom_color) {
        ImGui::PopStyleColor();
    }

    return value_changed;
}

bool imutils::toggle_button(const char* label, bool& value)
{
    assert(label);

    auto use_custom_color = !value;
    auto value_changed = false;

    if(use_custom_color) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_WindowBg));
    }

    if(ImGui::Button(label)) {
        value_changed = true;
        value = !value;
    }

    if(use_custom_color) {
        ImGui::PopStyleColor();
    }

    return value_changed;
}
