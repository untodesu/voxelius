#include "client/pch.hh"

#include "client/gui/slider.hh"

#include "core/config/map.hh"

template<typename T>
gui::Slider<T>& gui::Slider<T>::bind(config::Map& config, std::string_view key)
{
    this->set_key(key);

    m_value.bind(config, key);

    return this->self();
}

template<typename T>
gui::Slider<T>& gui::Slider<T>::set_range(T min, T max)
{
    m_min = min;
    m_max = max;

    return this->self();
}

template<typename T>
gui::Slider<T>& gui::Slider<T>::set_format(std::string_view format)
{
    m_format = format;

    return this->self();
}

template<>
void gui::Slider<int>::layout_control(void)
{
    auto current = m_value.value();
    auto format = m_format.size() ? m_format.c_str() : "%d";

    if(ImGui::SliderInt(this->imgui_id().c_str(), &current, m_min, m_max, format)) {
        m_value.set_value(current);
    }
}

template<>
void gui::Slider<float>::layout_control(void)
{
    auto current = m_value.value();
    auto format = m_format.size() ? m_format.c_str() : "%.3f";

    if(ImGui::SliderFloat(this->imgui_id().c_str(), &current, m_min, m_max, format)) {
        m_value.set_value(current);
    }
}

template<>
void gui::Slider<unsigned>::layout_control(void)
{
    auto current = m_value.value();
    auto format = m_format.size() ? m_format.c_str() : "%u";
    auto scalar = static_cast<std::uint32_t>(current);
    auto min = static_cast<std::uint32_t>(m_min);
    auto max = static_cast<std::uint32_t>(m_max);

    if(ImGui::SliderScalar(this->imgui_id().c_str(), ImGuiDataType_U32, &scalar, &min, &max, format)) {
        m_value.set_value(static_cast<unsigned>(scalar));
    }
}

template<typename T>
void gui::Slider<T>::translate_control(void)
{
    // empty
}

template class gui::Slider<int>;
template class gui::Slider<float>;
template class gui::Slider<unsigned>;
