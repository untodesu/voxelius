#include "client/pch.hh"

#include "client/gui/tabs.hh"

#include "client/globals.hh"

constexpr static float CALLBACK_GAP = 6.0f;

gui::Tabs& gui::Tabs::set_bar_height(float height)
{
    m_bar_height = height;

    return self();
}

gui::Tabs& gui::Tabs::set_callback(std::function<void(void)> callback)
{
    m_callback = std::move(callback);
    m_callback_label = std::format("<<{}", imgui_id());

    return self();
}

gui::Tabs& gui::Tabs::add_tab(std::string_view label_key, Widget& content, int priority)
{
    auto tab = std::make_shared<TabItem>();
    tab->content = &content;
    tab->priority = priority;

    tab->button.set_text(label_key);
    tab->button.on_click([this, tab] {
        select(tab);
    });

    auto it = std::ranges::upper_bound(m_tabs, priority, std::less {}, &TabItem::priority);
    auto inserted = m_tabs.insert(it, std::move(tab));

    if(1 == m_tabs.size()) {
        select(*inserted);
    }

    return self();
}

void gui::Tabs::select(std::shared_ptr<TabItem> target)
{
    for(auto& tab : m_tabs) {
        tab->button.set_pressed(tab == target);
    }

    m_selected = target;
}

void gui::Tabs::layout(void)
{
    auto bar_height = m_bar_height * static_cast<float>(globals::gui_scale);
    auto avail_size = ImGui::GetContentRegionAvail();

    if(m_callback) {
        auto callback_width = bar_height;
        auto callback_gap = CALLBACK_GAP * static_cast<float>(globals::gui_scale);

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.000f, 0.000f, 0.000f, 0.000f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.500f, 0.500f, 0.500f, 0.125f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.500f, 0.500f, 0.500f, 0.500f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);

        if(ImGui::Button(m_callback_label.c_str(), ImVec2(callback_width, bar_height))) {
            m_callback();
        }

        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);

        ImGui::SameLine(0.0f, callback_gap);
        avail_size.x -= callback_width + callback_gap;
    }

    auto button_width = 0.0f;

    if(m_tabs.size()) {
        button_width = std::max(1.0f, avail_size.x / static_cast<float>(m_tabs.size()));
    }

    for(std::size_t i = 0; i < m_tabs.size(); ++i) {
        ImGui::PushID(static_cast<int>(i));

        m_tabs[i]->button.set_size(button_width, bar_height);
        m_tabs[i]->button.layout();

        ImGui::PopID();

        if(i + 1 < m_tabs.size()) {
            ImGui::SameLine(0.0f, 0.0f);
        }
    }

    ImGui::Separator();

    if(m_selected && m_selected->content->visible()) {
        m_selected->content->layout();
    }
}

void gui::Tabs::translate(void)
{
    for(auto& tab : m_tabs) {
        tab->button.translate();
        tab->content->translate();
    }
}
