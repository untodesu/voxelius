#include "client/pch.hh"

#include "client/gui.hh"

#include "shared/world.hh"

#include "client/background.hh"
#include "client/constant.hh"
#include "client/fonts.hh"
#include "client/globals.hh"
#include "client/language.hh"
#include "client/main_menu.hh"
#include "client/settings.hh"
#include "client/style.hh"

gui_screen gui::screen = GUI_SCREEN_NONE;
unsigned gui::scale = 1U;

void gui::init(void)
{
    style::apply();
    fonts::load();

    language::init();

    background::init();

    main_menu::init();
    settings::init();
}

void gui::init_late(void)
{
    language::init_late();

    settings::init_late();

    gui::screen = GUI_MAIN_MENU;
}

void gui::shutdown(void)
{
    settings::shutdown();
    main_menu::shutdown();

    background::shutdown();
}

void gui::layout(void)
{
    if(world::basic_entities.valid(globals::player) && ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        switch(gui::screen) {
            case GUI_SCREEN_NONE:
                gui::screen = GUI_MAIN_MENU;
                break;

            case GUI_MAIN_MENU:
                gui::screen = GUI_SCREEN_NONE;
                break;
        }
    }

    background::layout();

    if(gui::screen) {
        switch(gui::screen) {
            case GUI_MAIN_MENU:
                main_menu::layout();
                break;

            case GUI_PLAY_MENU:
                // TODO: play_menu::layout();
                break;

            case GUI_SETTINGS:
                settings::layout();
                break;

            case GUI_PROGRESS:
                // TODO: progress::layout();
                break;

            case GUI_MESSAGE:
                // TODO: message::layout();
                break;

            case GUI_CONNECT:
                // TODO: connect::layout();
                break;
        }
    }
}

void gui::update_scale(void)
{
    int width, height;
    SDL_GetWindowSize(globals::window, &width, &height);

    auto scale_x = std::max(1.0f, std::floor(static_cast<float>(width) / static_cast<float>(constant::BASE_WIDTH)));
    auto scale_y = std::max(1.0f, std::floor(static_cast<float>(height) / static_cast<float>(constant::BASE_HEIGHT)));
    auto scale_min = std::min(scale_x, scale_y);

    auto& io = ImGui::GetIO();

    io.FontGlobalScale = scale_min;

    gui::scale = static_cast<unsigned>(scale_min);
}
