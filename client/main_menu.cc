#include "client/pch.hh"

#include "client/main_menu.hh"

#include "core/res/resource.hh"

#include "shared/world.hh"

#include "client/fonts.hh"
#include "client/globals.hh"
#include "client/gui.hh"
#include "client/language.hh"
#include "client/utils/imgui.hh"

static std::string str_button_play;
static std::string str_button_resume;
static std::string str_button_settings;
static std::string str_button_disconnect;
static std::string str_button_quit;

static std::string str_quit_popup_title;
static std::string str_quit_popup_question;
static std::array<std::string, 2> str_quit_popup_choices;

static void on_keyboard_event(const SDL_KeyboardEvent& event)
{
    if(world::basic_entities.valid(globals::player) && event.type == SDL_EVENT_KEY_DOWN && event.key == SDLK_ESCAPE) {
        if(gui::screen == GUI_SCREEN_NONE) {
            gui::screen = GUI_MAIN_MENU;
            return;
        }

        if(gui::screen == GUI_MAIN_MENU) {
            gui::screen = GUI_SCREEN_NONE;
            return;
        }
    }
}

static void on_language_update_event(const LanguageUpdateEvent& event)
{
    str_button_play = language::resolve_gui("main_menu.button.play");
    str_button_resume = language::resolve_gui("main_menu.button.resume");
    str_button_settings = language::resolve("main_menu.button.settings");
    str_button_disconnect = language::resolve("main_menu.button.disconnect");
    str_button_quit = language::resolve("main_menu.button.quit");

    str_quit_popup_title = language::resolve_gui("main_menu.quit_popup.title");
    str_quit_popup_question = language::resolve("main_menu.quit_popup.question");
    str_quit_popup_choices[0] = language::resolve_gui("main_menu.quit_popup.choice.yes");
    str_quit_popup_choices[1] = language::resolve_gui("main_menu.quit_popup.choice.no");
}

void main_menu::init(void)
{
    globals::dispatcher.sink<SDL_KeyboardEvent>().connect<&on_keyboard_event>();
    globals::dispatcher.sink<LanguageUpdateEvent>().connect<&on_language_update_event>();
}

void main_menu::shutdown(void)
{
    // empty
}

void main_menu::layout(void)
{
    const auto viewport = ImGui::GetMainViewport();
    const auto& viewport_size = viewport->Size;

    ImVec2 margin(gui::scale * 8.0f, gui::scale * 8.0f);
    ImVec2 control(gui::scale * 256.0f, gui::scale * 32.0f);

    ImGui::SetNextWindowPos({});
    ImGui::SetNextWindowSize(viewport_size);

    ImGuiWindowFlags flags = 0U;
    flags |= ImGuiWindowFlags_NoBackground;
    flags |= ImGuiWindowFlags_NoDecoration;

    if(!ImGui::Begin("###gui::main_menu", nullptr, flags)) {
        ImGui::End();
        return;
    }

    ImGui::PushFont(fonts::unscii16, 16.0f);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.000f, 0.000f, 0.000f, 0.000f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.500f, 0.500f, 0.500f, 0.125f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.500f, 0.500f, 0.500f, 0.500f));
    ImGui::PushStyleVarX(ImGuiStyleVar_FramePadding, 16.0f * gui::scale);
    ImGui::PushStyleVarX(ImGuiStyleVar_ButtonTextAlign, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);

    ImVec2 cursor_pos(margin.x, viewport_size.y - margin.y);

    cursor_pos.y -= control.y * 1.5f;
    ImGui::SetCursorPos(cursor_pos);

    if(world::basic_entities.valid(globals::player)) {
        utils::button(str_button_disconnect.c_str(), control, [] {
            LOG_INFO("TODO: disconnect");
        });
    }
    else {
        utils::button(str_button_quit.c_str(), control, [] {
            ImGui::OpenPopup(str_quit_popup_title.c_str());
        });
    }

    cursor_pos.y -= control.y;
    ImGui::SetCursorPos(cursor_pos);

    utils::button(str_button_settings.c_str(), control, [] {
        gui::screen = GUI_SETTINGS;
    });

    cursor_pos.y -= control.y * 1.5f;
    ImGui::SetCursorPos(cursor_pos);

    if(world::basic_entities.valid(globals::player)) {
        utils::button(str_button_resume.c_str(), control, [] {
            gui::screen = GUI_SCREEN_NONE;
        });
    }
    else {
        utils::button(str_button_play.c_str(), control, [] {
            gui::screen = GUI_PLAY_MENU;
        });
    }

    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(3);
    ImGui::PopFont();

    auto popup_result = utils::popup(str_quit_popup_title, str_quit_popup_question, str_quit_popup_choices);

    if(popup_result.has_value() && popup_result.value() == 0) {
        // We don't really have a good way to
        // pass "i want to quit" boolean to the main loop,
        // so instead we just raise an external interrupt signal
        // which handler latches an internal flag in the main loop
        std::raise(SIGINT);
    }

    ImGui::End();
}
