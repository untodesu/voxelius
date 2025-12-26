// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: main_menu.cc
// Description: Main menu screen

#include "client/pch.hh"

#include "client/gui/main_menu.hh"

#include "core/math/constexpr.hh"

#include "core/resource/resource.hh"

#include "core/version.hh"

#include "client/gui/gui_screen.hh"
#include "client/gui/imutils_button.hh"
#include "client/gui/imutils_popup.hh"
#include "client/gui/language.hh"
#include "client/gui/window_title.hh"

#include "client/io/glfw.hh"

#include "client/resource/texture_gui.hh"

#include "client/globals.hh"
#include "client/session.hh"

static std::string str_button_play;
static std::string str_button_resume;
static std::string str_button_settings;
static std::string str_button_disconnect;
static std::string str_button_quit;

static std::string str_quit_popup_title;
static std::string str_quit_popup_question;
static std::string str_quit_popup_choices[2];

static void on_glfw_key(const GlfwKeyEvent& event)
{
    if(session::is_ingame() && (event.key == GLFW_KEY_ESCAPE) && (event.action == GLFW_PRESS)) {
        if(globals::gui_screen == GUI_SCREEN_NONE) {
            globals::gui_screen = GUI_MAIN_MENU;
            return;
        }

        if(globals::gui_screen == GUI_MAIN_MENU) {
            globals::gui_screen = GUI_SCREEN_NONE;
            return;
        }
    }
}

static void on_language_set(const LanguageSetEvent& event)
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
    globals::dispatcher.sink<GlfwKeyEvent>().connect<&on_glfw_key>();
    globals::dispatcher.sink<LanguageSetEvent>().connect<&on_language_set>();
}

void main_menu::shutdown(void)
{
    // empty
}

void main_menu::layout(void)
{
    const auto viewport = ImGui::GetMainViewport();
    const auto& viewport_size = viewport->Size;

    ImVec2 margin(globals::gui_scale * 8.0f, globals::gui_scale * 8.0f);
    ImVec2 control(globals::gui_scale * 256.0f, globals::gui_scale * 32.0f);

    ImGui::SetNextWindowPos({});
    ImGui::SetNextWindowSize(viewport_size);

    ImGuiWindowFlags flags = 0U;
    flags |= ImGuiWindowFlags_NoBackground;
    flags |= ImGuiWindowFlags_NoDecoration;

    if(!ImGui::Begin("###gui::main_menu", nullptr, flags)) {
        ImGui::End();
        return;
    }

    ImGui::PushFont(globals::font_unscii16, 16.0f);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.000f, 0.000f, 0.000f, 0.000f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.500f, 0.500f, 0.500f, 0.125f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.500f, 0.500f, 0.500f, 0.500f));
    ImGui::PushStyleVarX(ImGuiStyleVar_FramePadding, 16.0f * globals::gui_scale);
    ImGui::PushStyleVarX(ImGuiStyleVar_ButtonTextAlign, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);

    ImVec2 cursor_pos(margin.x, viewport_size.y - margin.y);

    cursor_pos.y -= control.y * 1.5f;
    ImGui::SetCursorPos(cursor_pos);

    if(session::is_ingame()) {
        imutils::button(str_button_disconnect.c_str(), control, [] {
            session::disconnect("protocol.client_disconnect");
        });
    }
    else {
        imutils::button(str_button_quit.c_str(), control, [] {
            ImGui::OpenPopup(str_quit_popup_title.c_str());
        });
    }

    cursor_pos.y -= control.y;
    ImGui::SetCursorPos(cursor_pos);

    imutils::button(str_button_settings.c_str(), control, [] {
        globals::gui_screen = GUI_SETTINGS;
    });

    cursor_pos.y -= control.y * 1.5f;
    ImGui::SetCursorPos(cursor_pos);

    if(session::is_ingame()) {
        imutils::button(str_button_resume.c_str(), control, [] {
            globals::gui_screen = GUI_SCREEN_NONE;
        });
    }
    else {
        imutils::button(str_button_play.c_str(), control, [] {
            globals::gui_screen = GUI_PLAY_MENU;
        });
    }

    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(3);
    ImGui::PopFont();

    if(0 == imutils::popup(str_quit_popup_title, str_quit_popup_question, str_quit_popup_choices, 2)) {
        // We don't really have a good way to
        // pass "i want to quit" boolean to the main loop,
        // so instead we just raise an external interrupt signal
        // which handler latches an internal flag in the main loop
        std::raise(SIGINT);
    }

    ImGui::End();
}
