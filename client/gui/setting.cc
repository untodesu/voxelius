#include "client/pch.hh"

#include "client/gui/setting.hh"

#include "client/globals.hh"
#include "client/language.hh"

constexpr static float CONTROL_FRACTION = 0.6f;
constexpr static float ROW_PAD_X = 8.0f;

void gui::detail::SettingWidget::layout(void)
{
    auto pad_x = ROW_PAD_X * static_cast<float>(globals::gui_scale);

    auto row_start_x = ImGui::GetCursorPosX();
    auto content_size = ImGui::GetContentRegionAvail();
    auto row_min = ImGui::GetCursorScreenPos();

    auto draw_list = ImGui::GetWindowDrawList();
    draw_list->ChannelsSplit(2);
    draw_list->ChannelsSetCurrent(1);

    ImGui::SetCursorPosX(row_start_x + pad_x);
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(m_label.c_str());

    if(m_tooltip_enabled) {
        if(ImGui::BeginItemTooltip()) {
            ImGui::PushTextWrapPos(16.0f * ImGui::GetFontSize());
            ImGui::TextUnformatted(m_tooltip.c_str());
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
    }

    auto control_width = CONTROL_FRACTION * content_size.x - pad_x;

    ImGui::SetNextItemWidth(control_width);
    ImGui::SameLine(row_start_x + content_size.x - control_width - pad_x);

    layout_control();

    auto item_max = ImGui::GetItemRectMax();
    auto row_max = ImVec2(row_min.x + content_size.x, item_max.y);

    const auto& style = ImGui::GetStyle();
    const auto& item_spacing = style.ItemSpacing;

    auto row_pad_y = 0.5f * item_spacing.y;
    row_min.y -= row_pad_y;
    row_max.y += row_pad_y;

    draw_list->ChannelsSetCurrent(0);

    if(ImGui::IsMouseHoveringRect(row_min, row_max)) {
        draw_list->AddRectFilled(row_min, row_max, ImGui::GetColorU32(m_highlight));
    }

    draw_list->ChannelsMerge();
}

void gui::detail::SettingWidget::translate(void)
{
    m_label = language::resolve(std::format("settings.value.{}", m_key));
    m_tooltip = language::resolve(std::format("settings.tooltip.{}", m_key));
    translate_control();
}
