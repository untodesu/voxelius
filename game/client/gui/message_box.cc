#include "client/pch.hh"

#include "client/gui/message_box.hh"

#include "client/gui/gui_screen.hh"
#include "client/gui/language.hh"

#include "client/globals.hh"

constexpr static ImGuiWindowFlags WINDOW_FLAGS = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration;

struct Button final {
    gui::message_box_action action;
    std::string str_title;
};

static std::string str_title;
static std::string str_subtitle;
static std::vector<Button> buttons;

void gui::message_box::init(void)
{
    str_title = std::string();
    str_subtitle = std::string();
    buttons.clear();
}

void gui::message_box::layout(void)
{
    const auto viewport = ImGui::GetMainViewport();
    const auto window_start = ImVec2(0.0f, viewport->Size.y * 0.30f);
    const auto window_size = ImVec2(viewport->Size.x, viewport->Size.y * 0.70f);

    ImGui::SetNextWindowPos(window_start);
    ImGui::SetNextWindowSize(window_size);

    if(ImGui::Begin("###UIProgress", nullptr, WINDOW_FLAGS)) {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 1.0f * globals::gui_scale));

        const float title_width = ImGui::CalcTextSize(str_title.c_str()).x;
        ImGui::SetCursorPosX(0.5f * (window_size.x - title_width));
        ImGui::TextUnformatted(str_title.c_str());

        ImGui::Dummy(ImVec2(0.0f, 8.0f * globals::gui_scale));

        if(!str_subtitle.empty()) {
            const float subtitle_width = ImGui::CalcTextSize(str_subtitle.c_str()).x;
            ImGui::SetCursorPosX(0.5f * (window_size.x - subtitle_width));
            ImGui::TextUnformatted(str_subtitle.c_str());
        }

        ImGui::Dummy(ImVec2(0.0f, 32.0f * globals::gui_scale));

        for(const auto& button : buttons) {
            const float button_width = 0.8f * ImGui::CalcItemWidth();
            ImGui::SetCursorPosX(0.5f * (window_size.x - button_width));

            if(ImGui::Button(button.str_title.c_str(), ImVec2(button_width, 0.0f))) {
                if(button.action) {
                    button.action();
                }
            }
        }

        ImGui::PopStyleVar();
    }

    ImGui::End();
}

void gui::message_box::reset(void)
{
    str_title.clear();
    str_subtitle.clear();
    buttons.clear();
}

void gui::message_box::set_title(std::string_view title)
{
    str_title = gui::language::resolve(title);
}

void gui::message_box::set_subtitle(std::string_view subtitle)
{
    str_subtitle = gui::language::resolve(subtitle);
}

void gui::message_box::add_button(std::string_view text, const message_box_action& action)
{
    Button button = {};
    button.str_title = std::format("{}###MessageBox_Button{}", gui::language::resolve(text), buttons.size());
    button.action = action;

    buttons.push_back(button);
}
