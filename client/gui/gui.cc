#include "client/pch.hh"

#include "client/gui/gui.hh"

#include "client/gui/fonts.hh"
#include "client/gui/language.hh"
#include "client/gui/style.hh"

#include "client/constant.hh"
#include "client/globals.hh"

gui_screen gui::screen = GUI_SCREEN_NONE;
unsigned gui::scale = 1U;

void gui::init(void)
{
    detail::apply_style();
    detail::load_fonts();

    language::init();
}

void gui::init_late(void)
{
    language::init_late();
}

void gui::shutdown(void)
{
    // empty
}

void gui::layout(void)
{
    // empty
}

void gui::update_scale(void)
{
    int width, height;
    SDL_GetWindowSize(globals::window, &width, &height);

    auto scale_x = std::max(1.0f, std::floor(static_cast<float>(width) / static_cast<float>(constant::BASE_WIDTH)));
    auto scale_y = std::max(1.0f, std::floor(static_cast<float>(height) / static_cast<float>(constant::BASE_WIDTH)));
    auto scale_min = std::min(scale_x, scale_y);

    auto& io = ImGui::GetIO();

    io.FontGlobalScale = scale_min;

    gui::scale = static_cast<unsigned>(scale_min);
}
