#include "client/pch.hh"

#include "client/gui/stepper.hh"

#include "core/config/map.hh"

#include "client/language.hh"

template<typename T>
gui::Stepper<T>& gui::Stepper<T>::bind(config::Map& config, std::string_view key)
{
    this->set_key(key);

    m_value.bind(config, key);

    return this->self();
}

template<typename T>
gui::Stepper<T>& gui::Stepper<T>::set_range(T min, T max, T step)
{
    m_min = min;
    m_max = max;
    m_step = step;

    return this->self();
}

template<typename T>
T gui::Stepper<T>::value(void) const
{
    return m_value.value();
}

template<typename T>
void gui::Stepper<T>::set_value(T value)
{
    m_value.set_value(value);
}

template<typename T>
bool gui::Stepper<T>::dirty(void) const
{
    return m_value.dirty();
}

template<typename T>
void gui::Stepper<T>::layout_control(void)
{
    if(m_labels.empty()) {
        return;
    }

    auto current = m_value.value();
    auto index = static_cast<std::size_t>((current - m_min) / m_step);

    auto frame_height = ImGui::GetFrameHeight();
    auto combo_width = ImGui::CalcItemWidth() - 2.0f * frame_height;

    if(combo_width < 1.0f) {
        combo_width = 1.0f;
    }

    auto& control_id = this->imgui_id();

    ImGui::PushID(control_id.c_str());

    if(ImGui::Button("<", ImVec2(frame_height, frame_height))) {
        if(current <= m_min) {
            current = m_max;
        }
        else {
            current = static_cast<T>(current - m_step);
        }

        m_value.set_value(current);
    }

    ImGui::SameLine(0.0f, 0.0f);
    ImGui::SetNextItemWidth(combo_width);

    if(ImGui::BeginCombo("###combo", m_labels[index].c_str())) {
        for(std::size_t i = 0; i < m_labels.size(); ++i) {
            if(ImGui::Selectable(m_labels[i].c_str(), i == index)) {
                m_value.set_value(static_cast<T>(m_min + static_cast<T>(i) * m_step));
            }
        }

        ImGui::EndCombo();
    }

    ImGui::SameLine(0.0f, 0.0f);

    if(ImGui::Button(">", ImVec2(frame_height, frame_height))) {
        if(current >= m_max) {
            current = m_min;
        }
        else {
            current = static_cast<T>(current + m_step);
        }

        m_value.set_value(current);
    }

    ImGui::PopID();
}

template<typename T>
void gui::Stepper<T>::translate_control(void)
{
    auto count = static_cast<std::size_t>((m_max - m_min) / m_step) + 1;

    m_labels.resize(count);

    for(std::size_t i = 0; i < count; ++i) {
        m_labels[i] = language::resolve(std::format("settings.value.{}.{}", this->m_key, i));
    }
}

template class gui::Stepper<int>;
template class gui::Stepper<unsigned>;
