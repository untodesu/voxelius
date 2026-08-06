#include "client/pch.hh"

#include "client/gui/input.hh"

#include "core/config/map.hh"

template<typename T>
gui::Input<T>& gui::Input<T>::bind(config::Map& config, std::string_view key)
{
    this->set_key(key);

    m_value.bind(config, key);

    return this->self();
}

template<typename T>
gui::Input<T>& gui::Input<T>::set_range(T min, T max)
{
    m_min = min;
    m_max = max;

    return this->self();
}

template<>
void gui::Input<int>::layout_control(void)
{
    auto current = m_value.value();

    if(ImGui::InputInt(this->imgui_id().c_str(), &current)) {
        m_value.set_value(std::clamp(current, m_min, m_max));
    }
}

template<>
void gui::Input<float>::layout_control(void)
{
    auto current = m_value.value();

    if(ImGui::InputFloat(this->imgui_id().c_str(), &current, 0.0f, 0.0f, "%.3f")) {
        m_value.set_value(std::clamp(current, m_min, m_max));
    }
}

template<>
void gui::Input<unsigned>::layout_control(void)
{
    auto current = m_value.value();
    auto scalar = static_cast<std::uint32_t>(current);

    if(ImGui::InputScalar(this->imgui_id().c_str(), ImGuiDataType_U32, &scalar)) {
        m_value.set_value(std::clamp(static_cast<unsigned>(scalar), m_min, m_max));
    }
}

template<typename T>
void gui::Input<T>::translate_control(void)
{
    // empty
}

template class gui::Input<int>;
template class gui::Input<float>;
template class gui::Input<unsigned>;
