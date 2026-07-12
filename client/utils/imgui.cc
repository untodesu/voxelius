#include "client/pch.hh"

#include "client/utils/imgui.hh"

#include "client/fonts.hh"
#include "client/gui.hh"

void utils::text_nw(ImDrawList* list, std::string_view text, const ImVec2& pos, ImU32 color, ImU32 shadow, ImFont* font)
{
    assert(list);
    assert(font);

    text_nw(list, text, pos, color, shadow, font, font->LegacySize);
}

void utils::text_nw(ImDrawList* list, std::string_view text, const ImVec2& pos, ImU32 color, ImU32 shadow, ImFont* font, float size)
{
    assert(list);
    assert(font);

    auto shift = static_cast<float>(gui::scale);
    auto shadow_pos = ImVec2(pos.x + shift, pos.y + shift);

    list->AddText(font, size, shadow_pos, shadow, text.data(), text.data() + text.size());
    list->AddText(font, size, pos, color, text.data(), text.data() + text.size());
}

void utils::text_wr(ImDrawList* list, std::string_view text, const ImVec2& pos, ImU32 color, ImU32 shadow, ImFont* font, float wrap)
{
    assert(list);
    assert(font);

    text_wr(list, text, pos, color, shadow, font, wrap, font->LegacySize);
}

void utils::text_wr(ImDrawList* list, std::string_view text, const ImVec2& pos, ImU32 color, ImU32 shadow, ImFont* font, float wrap,
    float size)
{
    assert(list);
    assert(font);

    auto shift = static_cast<float>(gui::scale);
    auto shadow_pos = ImVec2(pos.x + shift, pos.y + shift);

    list->AddText(font, size, shadow_pos, shadow, text.data(), text.data() + text.size(), wrap);
    list->AddText(font, size, pos, color, text.data(), text.data() + text.size(), wrap);
}

std::optional<int> utils::popup(const std::string& title, const std::string& question, std::span<const std::string> choices,
    float font_scale)
{
    assert(choices.size());

    std::optional<int> result = std::nullopt;

    ImGui::PushFont(fonts::unscii16, fonts::unscii16->LegacySize);

    if(ImGui::BeginPopupModal(title.c_str(), nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize)) {
        const auto viewport = ImGui::GetMainViewport();
        const auto& viewport_size = viewport->Size;

        ImVec2 popup_size(ImGui::GetWindowSize());
        ImVec2 popup_pos(viewport_size.x * 0.5f - popup_size.x * 0.5f, viewport_size.y * 0.5f - popup_size.y * 0.5f);

        ImGui::PushTextWrapPos(popup_size.x);
        ImGui::TextUnformatted(question.c_str());
        ImGui::PopTextWrapPos();

        ImGui::NewLine();

        auto& style = ImGui::GetStyle();
        auto& spacing = style.ItemSpacing;
        ImVec2 button_size(0.5f * (ImGui::CalcItemWidth() - spacing.x), 0.0f);

        for(std::size_t i = 0; i < choices.size(); ++i) {
            if(ImGui::Button(choices[i].c_str(), button_size)) {
                result = static_cast<int>(i);
                ImGui::CloseCurrentPopup();
            }

            if((i + 1) % 2 == 1) {
                ImGui::SameLine();
            }
            else if(i + 1 < choices.size()) {
                ImGui::Dummy(ImVec2(0.0, spacing.y));
            }
        }

        ImGui::SetWindowPos(popup_pos, ImGuiCond_Always);

        ImGui::EndPopup();
    }

    ImGui::PopFont();

    return result;
}

void utils::button(const char* title, const ImVec2& size, void (*callback)(void))
{
    assert(title);
    assert(callback);

    if(ImGui::Button(title, size)) {
        callback();
    }
}

void utils::button(const char* title, void (*callback)(void))
{
    assert(title);
    assert(callback);

    if(ImGui::Button(title)) {
        callback();
    }
}

bool utils::selectable_button(const char* label, const ImVec2& size, bool value)
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

bool utils::toggle_button(const char* label, const ImVec2& size, bool& value)
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

bool utils::toggle_button(const char* label, bool& value)
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
