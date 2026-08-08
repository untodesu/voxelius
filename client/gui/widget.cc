#include "client/pch.hh"

#include "client/gui/widget.hh"

void gui::Widget::translate(void)
{
    // empty
}

void gui::Widget::set_visible(bool visible)
{
    m_visible = visible;
}

const std::string& gui::Widget::imgui_id(void) const
{
    if(this == m_last_this) {
        return m_imgui_id;
    }

    m_last_this = this;
    m_imgui_id = std::format("###{}", static_cast<const void*>(this));
    return m_imgui_id;
}
