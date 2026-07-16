#include "client/pch.hh"

#include "client/gui/background.hh"

#include "core/exception.hh"
#include "core/identifier.hh"
#include "core/res/resource.hh"

#include "shared/world/world.hh"

#include "client/constant.hh"
#include "client/globals.hh"
#include "client/gui/gui.hh"
#include "client/res/texture2D.hh"

constexpr static std::string_view TEXTURE_PATH = "textures/gui/background.png";

static res::handle<Texture2D> s_texture;

void background::init(void)
{
    s_texture = res::load<Texture2D>(Identifier::from_parts(constant::BUILTIN_NAME_SPACE, TEXTURE_PATH));
    vx::throw_if_fmt(s_texture == nullptr, "{}: load failed", TEXTURE_PATH);
}

void background::shutdown(void)
{
    s_texture.reset();
}

void background::layout(void)
{
    if(gui::screen) {
        auto viewport = ImGui::GetMainViewport();
        auto draw_list = ImGui::GetBackgroundDrawList();

        if(gui::screen == GUI_CHAT) {
            return; // chat draws its own bg
        }

        if(world::basic_entities.valid(globals::player)) {
            auto darken = ImGui::GetColorU32(ImVec4(0.00f, 0.00f, 0.00f, 0.75f));
            auto darker = ImGui::GetColorU32(ImVec4(0.00f, 0.00f, 0.00f, 0.95f));
            draw_list->AddRectFilledMultiColor({}, viewport->Size, darker, darken, darken, darker);
        }
        else {
            draw_list->AddImage(s_texture->imgui, {}, viewport->Size);
        }
    }
}
