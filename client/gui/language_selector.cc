#include "client/pch.hh"

#include "client/gui/language_selector.hh"

#include "client/language.hh"

void gui::LanguageSelector::layout_control(void)
{
    auto current = language::current();
    std::string current_label(current->endonym());

    if(ImGui::BeginCombo(imgui_id().c_str(), current_label.c_str())) {
        auto it = language::const_begin();

        while(it < language::const_end()) {
        }

        for(auto it = language::const_begin(); it != language::const_end(); it = std::next(it)) {
            if(ImGui::Selectable(it->display_cstr(), it == current)) {
                language::set(it);
            }
        }

        ImGui::EndCombo();
    }
}

void gui::LanguageSelector::translate_control(void)
{
    // empty
}
