// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: crosshair.cc
// Description: Crosshair rendering

#include "client/pch.hh"

#include "client/gui/crosshair.hh"

#include "core/math/constexpr.hh"

#include "core/resource/resource.hh"

#include "client/resource/texture_gui.hh"

#include "client/globals.hh"
#include "client/session.hh"

static resource_ptr<TextureGUI> texture;

void crosshair::init(void)
{
    texture = resource::load<TextureGUI>("textures/gui/hud_crosshair.png",
        TEXTURE_GUI_LOAD_CLAMP_S | TEXTURE_GUI_LOAD_CLAMP_T | TEXTURE_GUI_LOAD_VFLIP);

    if(texture == nullptr) {
        spdlog::critical("crosshair: texture load failed");
        std::terminate();
    }
}

void crosshair::shutdown(void)
{
    texture = nullptr;
}

void crosshair::layout(void)
{
    auto viewport = ImGui::GetMainViewport();
    auto draw_list = ImGui::GetForegroundDrawList();

    auto scaled_width = glm::max<int>(static_cast<int>(1.25f * texture->size.x), static_cast<int>(globals::gui_scale * texture->size.x));
    auto scaled_height = glm::max<int>(static_cast<int>(1.25f * texture->size.y), static_cast<int>(globals::gui_scale * texture->size.y));
    auto start = ImVec2(static_cast<int>(0.5f * viewport->Size.x) - (scaled_width / 2.0f),
        static_cast<float>(static_cast<int>(0.5f * viewport->Size.y) - (scaled_height / 2.0f)));
    auto end = ImVec2(start.x + scaled_width, start.y + scaled_height);
    draw_list->AddImage(texture->handle, start, end);
}
