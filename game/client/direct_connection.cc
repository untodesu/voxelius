#include "client/pch.hh"

#include "client/direct_connection.hh"

#include "core/config.hh"
#include "core/strtools.hh"

#include "shared/protocol.hh"

#include "client/game.hh"
#include "client/glfw.hh"
#include "client/globals.hh"
#include "client/gui_screen.hh"
#include "client/language.hh"
#include "client/session.hh"

constexpr static ImGuiWindowFlags WINDOW_FLAGS = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration;

static std::string str_title;
static std::string str_connect;
static std::string str_cancel;

static std::string str_hostname;
static std::string str_password;

static std::string direct_hostname;
static std::string direct_password;

static void on_glfw_key(const GlfwKeyEvent& event)
{
    if((event.key == GLFW_KEY_ESCAPE) && (event.action == GLFW_PRESS)) {
        if(globals::gui_screen == GUI_DIRECT_CONNECTION) {
            globals::gui_screen = GUI_PLAY_MENU;
            return;
        }
    }
}

static void on_language_set(const LanguageSetEvent& event)
{
    str_title = language::resolve("direct_connection.title");
    str_connect = language::resolve_gui("direct_connection.connect");
    str_cancel = language::resolve_gui("direct_connection.cancel");

    str_hostname = language::resolve("direct_connection.hostname");
    str_password = language::resolve("direct_connection.password");
}

static void connect_to_server(void)
{
    auto parts = strtools::split(direct_hostname, ":");
    std::string parsed_hostname;
    std::uint16_t parsed_port;

    if(!parts[0].empty()) {
        parsed_hostname = parts[0];
    } else {
        parsed_hostname = std::string("localhost");
    }

    if(parts.size() >= 2) {
        parsed_port = vx::clamp<std::uint16_t>(strtoul(parts[1].c_str(), nullptr, 10), 1024, UINT16_MAX);
    } else {
        parsed_port = protocol::PORT;
    }

    session::connect(parsed_hostname.c_str(), parsed_port, direct_password.c_str());
}

void direct_connection::init(void)
{
    globals::dispatcher.sink<GlfwKeyEvent>().connect<&on_glfw_key>();
    globals::dispatcher.sink<LanguageSetEvent>().connect<&on_language_set>();
}

void direct_connection::layout(void)
{
    auto viewport = ImGui::GetMainViewport();
    auto window_start = ImVec2(0.25f * viewport->Size.x, 0.20f * viewport->Size.y);
    auto window_size = ImVec2(0.50f * viewport->Size.x, 0.80f * viewport->Size.y);

    ImGui::SetNextWindowPos(window_start);
    ImGui::SetNextWindowSize(window_size);

    if(ImGui::Begin("###UIDirectConnect", nullptr, WINDOW_FLAGS)) {
        const float title_width = ImGui::CalcTextSize(str_title.c_str()).x;
        ImGui::SetCursorPosX(0.5f * (window_size.x - title_width));
        ImGui::TextUnformatted(str_title.c_str());

        ImGui::Dummy(ImVec2(0.0f, 16.0f * globals::gui_scale));

        ImGuiInputTextFlags hostname_flags = ImGuiInputTextFlags_CharsNoBlank;

        if(client_game::streamer_mode.get_value()) {
            // Hide server hostname to avoid things like
            // followers flooding the server that is streamed online
            hostname_flags |= ImGuiInputTextFlags_Password;
        }

        auto avail_width = ImGui::GetContentRegionAvail().x;

        ImGui::PushItemWidth(avail_width);

        ImGui::InputText("###UIDirectConnect_hostname", &direct_hostname, hostname_flags);

        if(ImGui::BeginItemTooltip()) {
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 16.0f);
            ImGui::TextUnformatted(str_hostname.c_str());
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }

        ImGui::InputText("###UIDirectConnect_password", &direct_password, ImGuiInputTextFlags_Password);

        if(ImGui::BeginItemTooltip()) {
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 16.0f);
            ImGui::TextUnformatted(str_password.c_str());
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }

        ImGui::PopItemWidth();

        ImGui::Dummy(ImVec2(0.0f, 4.0f * globals::gui_scale));

        ImGui::BeginDisabled(strtools::is_whitespace(direct_hostname));

        if(ImGui::Button(str_connect.c_str(), ImVec2(avail_width, 0.0f))) {
            connect_to_server();
        }

        ImGui::EndDisabled();

        if(ImGui::Button(str_cancel.c_str(), ImVec2(avail_width, 0.0f))) {
            globals::gui_screen = GUI_PLAY_MENU;
        }
    }

    ImGui::End();
}
