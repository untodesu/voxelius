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

gui::InputPopup& gui::InputPopup::set_title(std::string_view title)
{
    m_title_key = title;

    return self();
}

gui::InputPopup& gui::InputPopup::add_input(std::string_view label, ImGuiInputTextFlags flags,
    std::function<bool(std::string_view)> validator)
{
    Field field {};
    field.label_key = label;
    field.flags = flags;
    field.validator = std::move(validator);
    m_fields.emplace_back(std::move(field));

    return self();
}

gui::InputPopup& gui::InputPopup::on_submit(std::function<void(std::span<const std::string>)> callback)
{
    m_submit = std::move(callback);

    return self();
}

gui::InputPopup& gui::InputPopup::on_cancel(std::function<void(void)> callback)
{
    m_cancel = std::move(callback);

    return self();
}

void gui::InputPopup::layout(void)
{
    assert(m_fields.size());

    if(m_queued_open) {
        ImGui::OpenPopup(m_title.c_str());

        m_focus_first = true;
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
        bool enter_pressed = ImGui::IsKeyPressed(ImGuiKey_Enter);

        for(std::size_t i = 0; i < m_fields.size(); ++i) {
            auto& field = m_fields[i];

            if(m_focus_first && i == 0) {
                ImGui::SetKeyboardFocusHere();
            }

            ImGui::PushID(static_cast<int>(i));

            ImGui::TextUnformatted(field.label.c_str());
            ImGui::SetNextItemWidth(content_width);

            if(ImGui::InputText("###value", &field.buffer_2, field.flags)) {
                if(field.validator && !field.validator(field.buffer_2)) {
                    field.buffer_2 = field.buffer_1;
                }
                else {
                    field.buffer_1 = field.buffer_2;
                }
            }

            ImGui::PopID();
        }

        m_focus_first = false;

        ImGui::NewLine();

        ImVec2 button_size {};
        button_size.x = 0.5f * content_width - 0.5f * spacing.x;
        button_size.y = 0.0f;

        if(ImGui::Button(m_cancel_label.c_str(), button_size)) {
            if(m_cancel) {
                m_cancel();
            }

            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if(ImGui::Button(m_submit_label.c_str(), button_size) || enter_pressed) {
            if(m_submit) {
                std::vector<std::string> values;
                values.reserve(m_fields.size());

                for(const auto& field : m_fields) {
                    values.push_back(field.buffer_1);
                }

                m_submit(values);
            }

            ImGui::CloseCurrentPopup();
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

void gui::InputPopup::translate(void)
{
    m_title = language::resolve(m_title_key);
    m_title += imgui_id();

    for(auto& field : m_fields) {
        field.label = language::resolve(field.label_key);
    }

    m_submit_label = language::resolve("gui.input_popup.submit");
    m_submit_label += std::format("{}[submit]", imgui_id());

    m_cancel_label = language::resolve("gui.input_popup.cancel");
    m_cancel_label += std::format("{}[cancel]", imgui_id());
}

void gui::InputPopup::open(void)
{
    m_queued_open = true;
}

void gui::InputPopup::set_value(std::size_t index, std::string_view value)
{
    m_fields[index].buffer_1 = value;
    m_fields[index].buffer_2 = value;
}

std::string_view gui::InputPopup::value(std::size_t index) const
{
    return m_fields[index].buffer_1;
}
