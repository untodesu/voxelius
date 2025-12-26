// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: imutils_popup.cc
// Description: Popup utilities

#include "client/pch.hh"

#include "client/gui/imutils_popup.hh"

#include "client/globals.hh"

int imutils::popup(const std::string& title, const std::string& question, const std::string* choices, std::size_t num_choices,
    float font_scale)
{
    assert(choices);
    assert(num_choices);

    int result = POPUP_WAIT;

    ImGui::PushFont(globals::font_unscii16, globals::font_unscii16->LegacySize);

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

        for(std::size_t i = 0; i < num_choices; ++i) {
            if(ImGui::Button(choices[i].c_str(), button_size)) {
                result = static_cast<int>(i);
                ImGui::CloseCurrentPopup();
            }

            if((i + 1) % 2 == 1) {
                ImGui::SameLine();
            }
            else if(i + 1 < num_choices) {
                ImGui::Dummy(ImVec2(0.0, spacing.y));
            }
        }

        ImGui::SetWindowPos(popup_pos, ImGuiCond_Always);

        ImGui::EndPopup();
    }

    ImGui::PopFont();

    return result;
}
