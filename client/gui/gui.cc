#include "client/pch.hh"

#include "client/gui/gui.hh"

#include "shared/world.hh"

#include "client/constant.hh"
#include "client/globals.hh"
#include "client/gui/background.hh"
#include "client/gui/fonts.hh"
#include "client/gui/language.hh"
#include "client/gui/main_menu.hh"
#include "client/gui/settings.hh"
#include "client/gui/style.hh"

gui_screen gui::screen = GUI_SCREEN_NONE;
unsigned gui::scale = 1U;

void gui::init(void)
{
    detail::apply_style();
    detail::load_fonts();

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
    if(!world::basic_entities.valid(globals::player)) {
        background::layout();
    }

    if(gui::screen) {
        if(world::basic_entities.valid(globals::player) && gui::screen != GUI_CHAT) {
            auto width_float = static_cast<float>(globals::width);
            auto height_float = static_cast<float>(globals::height);
            auto darken = ImGui::GetColorU32(ImVec4(0.00f, 0.00f, 0.00f, 0.75f));
            auto darker = ImGui::GetColorU32(ImVec4(0.00f, 0.00f, 0.00f, 0.95f));
            auto draw_list = ImGui::GetBackgroundDrawList();

            draw_list->AddRectFilledMultiColor({}, { width_float, height_float }, darker, darken, darken, darker);
        }

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
