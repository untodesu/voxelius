#include "client/pch.hh"

#include "client/gui/crosshair.hh"

#include "core/math/constexpr.hh"

#include "core/resource/resource.hh"

#include "client/resource/texture_gui.hh"

#include "client/globals.hh"
#include "client/session.hh"

static resource_ptr<TextureGUI> texture;

void gui::crosshair::init(void)
{
    texture = resource::load<TextureGUI>(
        "textures/gui/hud_crosshair.png", TEXTURE_GUI_LOAD_CLAMP_S | TEXTURE_GUI_LOAD_CLAMP_T | TEXTURE_GUI_LOAD_VFLIP);

    if(texture == nullptr) {
        spdlog::critical("crosshair: texture load failed");
        std::terminate();
    }
}

void gui::crosshair::shutdown(void)
{
    texture = nullptr;
}

void gui::crosshair::layout(void)
{
    auto viewport = ImGui::GetMainViewport();
    auto draw_list = ImGui::GetForegroundDrawList();

    auto scaled_width = math::max<int>(texture->size.x, globals::gui_scale * texture->size.x / 2);
    auto scaled_height = math::max<int>(texture->size.y, globals::gui_scale * texture->size.y / 2);
    auto start = ImVec2(
        static_cast<int>(0.5f * viewport->Size.x) - (scaled_width / 2), static_cast<int>(0.5f * viewport->Size.y) - (scaled_height / 2));
    auto end = ImVec2(start.x + scaled_width, start.y + scaled_height);
    draw_list->AddImage(texture->handle, start, end);
}
