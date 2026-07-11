#include "client/pch.hh"

#include "client/gui/background.hh"

#include "core/exception.hh"
#include "core/identifier.hh"
#include "core/res/resource.hh"

#include "client/constant.hh"
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
    auto viewport = ImGui::GetMainViewport();
    auto draw_list = ImGui::GetBackgroundDrawList();
    draw_list->AddImage(s_texture->imgui, {}, viewport->Size);
}
