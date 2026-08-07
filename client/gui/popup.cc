#include "client/pch.hh"

#include "client/gui/popup.hh"

#include "client/fonts.hh"
#include "client/language.hh"

gui::Popup& gui::Popup::set_title(std::string_view title)
{
    m_title_key = title;

    return self();
}

gui::Popup& gui::Popup::set_message(std::string_view question)
{
    m_message_key = question;

    return self();
}

gui::Popup& gui::Popup::add_choice(std::string_view choice, std::function<void(void)> callback)
{
    Choice item {};
    item.label_key = choice;
    item.callback = std::move(callback);
    m_choices.emplace_back(std::move(item));

    return self();
}

void gui::Popup::layout(void)
{
    assert(m_choices.size());

    if(m_queued_open) {
        ImGui::OpenPopup(m_title.c_str());

        m_queued_open = false;
    }

    auto window_pos = ImGui::GetWindowPos();
    auto window_size = ImGui::GetWindowSize();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
    ImGui::PushFont(fonts::unscii16, fonts::unscii16->LegacySize);

    if(ImGui::BeginPopupModal(m_title.c_str(), nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize)) {
        auto& style = ImGui::GetStyle();
        auto& spacing = style.ItemSpacing;

        auto content_width = ImGui::CalcItemWidth();
        ImGui::PushTextWrapPos(content_width + ImGui::GetCursorPosX());
        ImGui::TextUnformatted(m_message.c_str());
        ImGui::PopTextWrapPos();

        ImGui::NewLine();

        ImVec2 button_size {};
        button_size.x = 0.5f * content_width - 0.5f * spacing.x;
        button_size.y = 0.0;

        for(std::size_t i = 0; i < m_choices.size(); ++i) {
            const auto& choice = m_choices[i];

            if(ImGui::Button(choice.label.c_str(), button_size)) {
                if(choice.callback) {
                    choice.callback();
                }

                ImGui::CloseCurrentPopup();
            }

            if((i + 1) % 2 == 1) {
                ImGui::SameLine();
            }
            else if(i + 1 < m_choices.size()) {
                ImGui::Dummy(ImVec2(0.0, spacing.y));
            }
        }

        ImVec2 popup_size = ImGui::GetWindowSize();

        ImVec2 popup_pos {};
        popup_pos.x = 0.5f * window_size.x - 0.5f * popup_size.x + window_pos.x;
        popup_pos.y = 0.5f * window_size.y - 0.5f * popup_size.y + window_pos.y;

        ImGui::SetWindowPos(popup_pos, ImGuiCond_Always);

        ImGui::EndPopup();
    }

    ImGui::PopFont();
    ImGui::PopStyleVar();
}

void gui::Popup::translate(void)
{
    m_title = language::resolve(m_title_key);
    m_title += imgui_id();

    m_message = language::resolve(m_message_key);

    for(std::size_t i = 0; i < m_choices.size(); ++i) {
        m_choices[i].label = language::resolve(m_choices[i].label_key);
        m_choices[i].label += std::format("{}[{}]", imgui_id(), i);
    }
}

void gui::Popup::open(void)
{
    m_queued_open = true;
}
