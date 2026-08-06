#include "client/pch.hh"

#include "client/gui/setting.hh"

#include "client/language.hh"

constexpr static float CONTROL_FRACTION = 0.4f;

void gui::detail::SettingWidget::layout(void)
{
    ImGui::TextUnformatted(m_label.c_str());

    if(m_tooltip_enabled) {
        if(ImGui::BeginItemTooltip()) {
            ImGui::PushTextWrapPos(16.0f * ImGui::GetFontSize());
            ImGui::TextUnformatted(m_tooltip.c_str());
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
    }

    auto& style = ImGui::GetStyle();
    auto window_width = ImGui::GetWindowWidth();
    auto content_width = window_width - 2.0f * style.WindowPadding.x;
    auto control_width = CONTROL_FRACTION * content_width;

    ImGui::SetNextItemWidth(control_width);
    ImGui::SameLine(window_width - style.WindowPadding.x - control_width);

    layout_control();
}

void gui::detail::SettingWidget::translate(void)
{
    m_label = language::resolve(std::format("settings.value.{}", m_key));
    m_tooltip = language::resolve(std::format("settings.tooltip.{}", m_key));
    translate_control();
}
