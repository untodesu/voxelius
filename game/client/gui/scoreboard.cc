#include "client/pch.hh"

#include "client/gui/scoreboard.hh"

#include "core/io/config_map.hh"

#include "shared/protocol.hh"

#include "client/config/keybind.hh"
#include "client/gui/gui_screen.hh"
#include "client/gui/settings.hh"

#include "client/globals.hh"
#include "client/session.hh"

constexpr static ImGuiWindowFlags WINDOW_FLAGS =
    ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoBackground;

static config::KeyBind list_key(GLFW_KEY_TAB);

static std::vector<std::string> usernames;
static float max_username_size;

static void on_scoreboard_update_packet(const protocol::ScoreboardUpdate& packet)
{
    usernames = packet.names;
    max_username_size = 0.0f;
}

void gui::scoreboard::init(void)
{
    globals::client_config.add_value("scoreboard.key", list_key);

    settings::add_keybind(3, list_key, settings_location::KEYBOARD_MISC, "key.scoreboard");

    globals::dispatcher.sink<protocol::ScoreboardUpdate>().connect<&on_scoreboard_update_packet>();
}

void gui::scoreboard::layout(void)
{
    if(globals::gui_screen == GUI_SCREEN_NONE && session::is_ingame() && glfwGetKey(globals::window, list_key.get_key()) == GLFW_PRESS) {
        const auto viewport = ImGui::GetMainViewport();
        const auto window_start = ImVec2(0.0f, 0.0f);
        const auto window_size = ImVec2(viewport->Size.x, viewport->Size.y);

        ImGui::SetNextWindowPos(window_start);
        ImGui::SetNextWindowSize(window_size);

        if(!ImGui::Begin("###chat", nullptr, WINDOW_FLAGS)) {
            ImGui::End();
            return;
        }

        ImGui::PushFont(globals::font_chat);

        const auto& padding = ImGui::GetStyle().FramePadding;
        const auto& spacing = ImGui::GetStyle().ItemSpacing;
        auto font = globals::font_chat;

        // Figure out the maximum username size
        for(const auto& username : usernames) {
            const ImVec2 size = ImGui::CalcTextSize(username.c_str(), username.c_str() + username.size());

            if(size.x > max_username_size) {
                max_username_size = size.x;
            }
        }

        // Having a minimum size allows for
        // generally better in-game visibility
        const float true_size = math::max<float>(0.25f * window_size.x, max_username_size);

        // Figure out username rect dimensions
        const float rect_start_x = 0.5f * window_size.x - 0.5f * true_size;
        const float rect_start_y = 0.15f * window_size.y;
        const float rect_size_x = 2.0f * padding.x + true_size;
        const float rect_size_y = 2.0f * padding.y + font->FontSize;

        // const ImU32 border_col = ImGui::GetColorU32(ImGuiCol_Border, 1.00f);
        const ImU32 rect_col = ImGui::GetColorU32(ImGuiCol_FrameBg, 0.80f);
        const ImU32 text_col = ImGui::GetColorU32(ImGuiCol_Text, 1.00f);

        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        // Slightly space apart individual rows
        const float row_step_y = rect_size_y + 0.5f * spacing.y;

        for(std::size_t i = 0; i < usernames.size(); ++i) {
            const ImVec2 rect_a = ImVec2(rect_start_x, rect_start_y + i * row_step_y);
            const ImVec2 rect_b = ImVec2(rect_a.x + rect_size_x, rect_a.y + rect_size_y);
            const ImVec2 text_pos = ImVec2(rect_a.x + padding.x, rect_a.y + padding.y);

            // draw_list->AddRect(rect_a, rect_b, border_col, 0.0f, ImDrawFlags_None, globals::gui_scale);
            draw_list->AddRectFilled(rect_a, rect_b, rect_col, 0.0f, ImDrawFlags_None);
            draw_list->AddText(font, font->FontSize, text_pos, text_col, usernames[i].c_str(), usernames[i].c_str() + usernames[i].size());
        }

        ImGui::PopFont();
        ImGui::End();
    }
}
