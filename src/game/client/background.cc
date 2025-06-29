#include "client/pch.hh"

#include "client/background.hh"

#include "core/constexpr.hh"
#include "core/resource.hh"

#include "client/globals.hh"
#include "client/texture_gui.hh"

static resource_ptr<TextureGUI> texture;

void background::init(void)
{
    texture = resource::load<TextureGUI>("textures/gui/background.png", TEXTURE_GUI_LOAD_VFLIP);

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

    auto scaled_width = 0.75f * static_cast<float>(globals::width / globals::gui_scale);
    auto scaled_height = 0.75f * static_cast<float>(globals::height / globals::gui_scale);
    auto scale_uv = ImVec2(scaled_width / static_cast<float>(texture->size.x), scaled_height / static_cast<float>(texture->size.y));
    draw_list->AddImage(texture->handle, ImVec2(0.0f, 0.0f), viewport->Size, ImVec2(0.0f, 0.0f), scale_uv);
}
