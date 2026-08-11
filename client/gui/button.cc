#include "client/pch.hh"

#include "client/gui/button.hh"

#include "client/language.hh"

gui::Button& gui::Button::set_text(std::string_view key)
{
    m_key = key;

    return self();
}

gui::Button& gui::Button::set_size(float wide, float tall)
{
    m_size = ImVec2(wide, tall);

    return self();
}

gui::Button& gui::Button::on_click(std::function<void(void)> callback)
{
    m_callback = std::move(callback);

    return self();
}

gui::Button& gui::Button::set_enabled(bool enabled)
{
    m_enabled = enabled;

    return self();
}

void gui::Button::layout(void)
{
    ImGui::BeginDisabled(!m_enabled);

    if(ImGui::Button(m_text.c_str(), m_size)) {
        if(m_callback) {
            m_callback();
        }
    }

    ImGui::EndDisabled();
}

void gui::Button::translate(void)
{
    m_text = language::resolve(m_key);
    m_text.append(imgui_id());
}
