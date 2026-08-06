#include "client/pch.hh"

#include "client/gui/keybind.hh"

#include "core/config/map.hh"

#include "client/globals.hh"

gui::KeyBind* gui::KeyBind::current = nullptr;

void gui::KeyBind::init(void)
{
    globals::dispatcher.sink<SDL_KeyboardEvent>().connect<&gui::KeyBind::on_keyboard_event>();
}

void gui::KeyBind::layout_control(void)
{
    auto label = m_bound_label.c_str();

    if(current == this) {
        label = m_capture_label.c_str();
    }

    if(ImGui::Button(label, ImVec2(ImGui::CalcItemWidth(), 0.0f))) {
        ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;

        current = this;
    }
}

void gui::KeyBind::translate_control(void)
{
    m_capture_label = std::format("...{}", imgui_id());
    m_bound_label = std::format("{}{}", SDL_GetKeyName(m_value.value()), imgui_id());
}

void gui::KeyBind::on_keyboard_event(const SDL_KeyboardEvent& event)
{
    if(current && event.type == SDL_EVENT_KEY_DOWN) {
        auto& io = ImGui::GetIO();

        if(event.key != SDLK_ESCAPE) {
            current->m_value.set_value(event.key);
            current->m_bound_label = std::format("{}{}", SDL_GetKeyName(event.key), current->imgui_id());
        }

        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        current = nullptr;
    }
}
