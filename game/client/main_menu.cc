#include "client/pch.hh"

#include "client/main_menu.hh"

#include "core/constexpr.hh"
#include "core/resource.hh"
#include "core/version.hh"

#include "client/glfw.hh"
#include "client/globals.hh"
#include "client/gui_screen.hh"
#include "client/language.hh"
#include "client/session.hh"
#include "client/texture_gui.hh"
#include "client/window_title.hh"

constexpr static ImGuiWindowFlags WINDOW_FLAGS = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration;

static std::string str_play;
static std::string str_resume;
static std::string str_settings;
static std::string str_leave;
static std::string str_quit;

static resource_ptr<TextureGUI> title;
static float title_aspect;

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
    str_play = language::resolve_gui("main_menu.play");
    str_resume = language::resolve_gui("main_menu.resume");
    str_settings = language::resolve("main_menu.settings");
    str_leave = language::resolve("main_menu.leave");
    str_quit = language::resolve("main_menu.quit");
}

void main_menu::init(void)
{
    title = resource::load<TextureGUI>("textures/gui/menu_title.png", TEXTURE_GUI_LOAD_CLAMP_S | TEXTURE_GUI_LOAD_CLAMP_T);

    if(title == nullptr) {
        spdlog::critical("main_menu: texture load failed");
        std::terminate();
    }

    if(title->size.x > title->size.y) {
        title_aspect = static_cast<float>(title->size.x) / static_cast<float>(title->size.y);
    } else {
        title_aspect = static_cast<float>(title->size.y) / static_cast<float>(title->size.x);
    }

    globals::dispatcher.sink<GlfwKeyEvent>().connect<&on_glfw_key>();
    globals::dispatcher.sink<LanguageSetEvent>().connect<&on_language_set>();
}

void main_menu::deinit(void)
{
    title = nullptr;
}

void main_menu::layout(void)
{
    const auto viewport = ImGui::GetMainViewport();
    const auto window_start = ImVec2(0.0f, viewport->Size.y * 0.15f);
    const auto window_size = ImVec2(viewport->Size.x, viewport->Size.y);

    ImGui::SetNextWindowPos(window_start);
    ImGui::SetNextWindowSize(window_size);

    if(ImGui::Begin("###main_menu", nullptr, WINDOW_FLAGS)) {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 2.0f * globals::gui_scale));

        if(session::is_ingame()) {
            ImGui::Dummy(ImVec2(0.0f, 32.0f * globals::gui_scale));
        } else {
            auto reference_height = 0.225f * window_size.y;
            auto image_width = cxpr::min(window_size.x, reference_height * title_aspect);
            auto image_height = image_width / title_aspect;
            ImGui::SetCursorPosX(0.5f * (window_size.x - image_width));
            ImGui::Image(title->handle, ImVec2(image_width, image_height));
        }

        ImGui::Dummy(ImVec2(0.0f, 24.0f * globals::gui_scale));

        const float button_width = 240.0f * globals::gui_scale;
        const float button_xpos = 0.5f * (window_size.x - button_width);

        if(session::is_ingame()) {
            ImGui::SetCursorPosX(button_xpos);

            if(ImGui::Button(str_resume.c_str(), ImVec2(button_width, 0.0f))) {
                globals::gui_screen = GUI_SCREEN_NONE;
            }

            ImGui::Spacing();
        } else {
            ImGui::SetCursorPosX(button_xpos);

            if(ImGui::Button(str_play.c_str(), ImVec2(button_width, 0.0f))) {
                globals::gui_screen = GUI_PLAY_MENU;
            }

            ImGui::Spacing();
        }

        ImGui::SetCursorPosX(button_xpos);

        if(ImGui::Button(str_settings.c_str(), ImVec2(button_width, 0.0f))) {
            globals::gui_screen = GUI_SETTINGS;
        }

        ImGui::Spacing();

        if(session::is_ingame()) {
            ImGui::SetCursorPosX(button_xpos);

            if(ImGui::Button(str_leave.c_str(), ImVec2(button_width, 0.0f))) {
                session::disconnect("protocol.client_disconnect");
                globals::gui_screen = GUI_PLAY_MENU;
                window_title::update();
            }

            ImGui::Spacing();
        } else {
            ImGui::SetCursorPosX(button_xpos);

            if(ImGui::Button(str_quit.c_str(), ImVec2(button_width, 0.0f))) {
                glfwSetWindowShouldClose(globals::window, true);
            }

            ImGui::Spacing();
        }

        if(!session::is_ingame()) {
            const auto& padding = ImGui::GetStyle().FramePadding;
            const auto& spacing = ImGui::GetStyle().ItemSpacing;

            ImGui::PushFont(globals::font_debug);
            ImGui::SetCursorScreenPos(ImVec2(padding.x + spacing.x, window_size.y - globals::font_debug->FontSize - padding.y - spacing.y));
            ImGui::Text("Voxelius %s", PROJECT_VERSION_STRING);
            ImGui::PopFont();
        }

        ImGui::PopStyleVar();
    }

    ImGui::End();
}
