#include "client/pch.hh"

#include "client/gui/text_box.hh"

#include "core/cvar/host.hh"

gui::TextBox& gui::TextBox::bind(std::string_view name)
{
    this->set_key(name);

    cvar::bind_var(m_value, name);

    return this->self();
}

gui::TextBox& gui::TextBox::set_callback(ImGuiInputTextCallback validator, void* user_data)
{
    m_callback = validator;
    m_callback_user_data = user_data;

    return this->self();
}

gui::TextBox& gui::TextBox::set_flags(ImGuiInputTextFlags flags)
{
    m_flags = flags;

    return this->self();
}

std::string_view gui::TextBox::value(void) const
{
    return m_value.value();
}

void gui::TextBox::set_value(std::string_view value)
{
    m_value.set_value(std::string(value));
}

void gui::TextBox::layout_control(void)
{
    auto current = std::string(m_value.value());
    auto flags = m_flags;

    if(m_callback) {
        flags |= ImGuiInputTextFlags_CallbackCharFilter;
    }

    if(ImGui::InputText(this->imgui_id().c_str(), &current, flags, m_callback, m_callback_user_data)) {
        m_value.set_value(current);
    }
}

void gui::TextBox::translate_control(void)
{
    // empty
}
