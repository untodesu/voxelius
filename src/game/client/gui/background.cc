#include "client/pch.hh"

#include "client/gui/background.hh"

#include "core/math/constexpr.hh"

#include "core/resource/resource.hh"

#include "client/resource/texture_gui.hh"

#include "client/globals.hh"

static resource_ptr<TextureGUI> texture;

void background::init(void)
{
    texture = resource::load<TextureGUI>("textures/gui/background.png");

    if(texture == nullptr) {
        spdlog::critical("background: texture load failed");
        std::terminate();
    }
}

void background::shutdown(void)
{
    texture = nullptr;
}

void background::layout(void)
{
    auto viewport = ImGui::GetMainViewport();
    auto draw_list = ImGui::GetBackgroundDrawList();
    draw_list->AddImage(texture->handle, {}, viewport->Size);
}
