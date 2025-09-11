#include "client/pch.hh"

#include "client/gui/chat.hh"

#include "core/config/number.hh"
#include "core/config/string.hh"

#include "core/io/config_map.hh"

#include "core/resource/resource.hh"

#include "core/utils/string.hh"

#include "shared/protocol.hh"

#include "client/config/keybind.hh"

#include "client/gui/gui_screen.hh"
#include "client/gui/imdraw_ext.hh"
#include "client/gui/language.hh"
#include "client/gui/settings.hh"

#include "client/io/glfw.hh"

#include "client/resource/sound_effect.hh"

#include "client/sound/sound.hh"

#include "client/game.hh"
#include "client/globals.hh"
#include "client/session.hh"

constexpr static ImGuiWindowFlags WINDOW_FLAGS = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration;
constexpr static unsigned int MAX_HISTORY_SIZE = 128U;

struct GuiChatMessage final {
    std::uint64_t spawn;
    std::string text;
    ImVec4 color;
};

static config::KeyBind key_chat(GLFW_KEY_ENTER);
static config::Unsigned history_size(32U, 0U, MAX_HISTORY_SIZE);

static std::deque<GuiChatMessage> history;
static std::string chat_input;
static bool needs_focus;

static resource_ptr<SoundEffect> sfx_chat_message;

static void append_text_message(const std::string& sender, const std::string& text)
{
    GuiChatMessage message;
    message.spawn = globals::curtime;
    message.text = std::format("<{}> {}", sender, text);
    message.color = ImGui::GetStyleColorVec4(ImGuiCol_Text);
    history.push_back(message);

    if(sfx_chat_message && session::is_ingame()) {
        sound::play_ui(sfx_chat_message, false, 1.0f);
    }
}

static void append_player_join(const std::string& sender)
{
    GuiChatMessage message;
    message.spawn = globals::curtime;
    message.text = std::format("{} {}", sender, gui::language::resolve("chat.client_join"));
    message.color = ImGui::GetStyleColorVec4(ImGuiCol_DragDropTarget);
    history.push_back(message);

    if(sfx_chat_message && session::is_ingame()) {
        sound::play_ui(sfx_chat_message, false, 1.0f);
    }
}

static void append_player_leave(const std::string& sender, const std::string& reason)
{
    GuiChatMessage message;
    message.spawn = globals::curtime;
    message.text = std::format("{} {} ({})", sender, gui::language::resolve("chat.client_left"), gui::language::resolve(reason.c_str()));
    message.color = ImGui::GetStyleColorVec4(ImGuiCol_DragDropTarget);
    history.push_back(message);

    if(sfx_chat_message && session::is_ingame()) {
        sound::play_ui(sfx_chat_message, false, 1.0f);
    }
}

static void on_chat_message_packet(const protocol::ChatMessage& packet)
{
    if(packet.type == protocol::ChatMessage::TEXT_MESSAGE) {
        append_text_message(packet.sender, packet.message);
        return;
    }

    if(packet.type == protocol::ChatMessage::PLAYER_JOIN) {
        append_player_join(packet.sender);
        return;
    }

    if(packet.type == protocol::ChatMessage::PLAYER_LEAVE) {
        append_player_leave(packet.sender, packet.message);
        return;
    }
}

static void on_glfw_key(const io::GlfwKeyEvent& event)
{
    if(event.action == GLFW_PRESS) {
        if((event.key == GLFW_KEY_ENTER) && (globals::gui_screen == GUI_CHAT)) {
            if(!utils::is_whitespace(chat_input)) {
                protocol::ChatMessage packet;
                packet.type = protocol::ChatMessage::TEXT_MESSAGE;
                packet.sender = client_game::username.get();
                packet.message = chat_input;

                protocol::send(session::peer, protocol::encode(packet));
            }

            globals::gui_screen = GUI_SCREEN_NONE;

            chat_input.clear();

            return;
        }

        if((event.key == GLFW_KEY_ESCAPE) && (globals::gui_screen == GUI_CHAT)) {
            globals::gui_screen = GUI_SCREEN_NONE;
            return;
        }

        if(key_chat.equals(event.key) && !globals::gui_screen) {
            globals::gui_screen = GUI_CHAT;
            needs_focus = true;
            return;
        }
    }
}

void gui::client_chat::init(void)
{
    globals::client_config.add_value("chat.key", key_chat);
    globals::client_config.add_value("chat.history_size", history_size);

    settings::add_keybind(2, key_chat, settings_location::KEYBOARD_MISC, "key.chat");
    settings::add_slider(1, history_size, settings_location::VIDEO_GUI, "chat.history_size", false);

    globals::dispatcher.sink<protocol::ChatMessage>().connect<&on_chat_message_packet>();
    globals::dispatcher.sink<io::GlfwKeyEvent>().connect<&on_glfw_key>();

    sfx_chat_message = resource::load<SoundEffect>("sounds/ui/chat_message.wav");
}

void gui::client_chat::init_late(void)
{
}

void gui::client_chat::shutdown(void)
{
    sfx_chat_message = nullptr;
}

void gui::client_chat::update(void)
{
    while(history.size() > history_size.get_value()) {
        history.pop_front();
    }
}

void gui::client_chat::layout(void)
{
    auto viewport = ImGui::GetMainViewport();
    auto window_start = ImVec2(0.0f, 0.0f);
    auto window_size = ImVec2(0.75f * viewport->Size.x, viewport->Size.y);

    ImGui::SetNextWindowPos(window_start);
    ImGui::SetNextWindowSize(window_size);

    ImGui::PushFont(globals::font_unscii16, 8.0f);

    if(!ImGui::Begin("###chat", nullptr, WINDOW_FLAGS)) {
        ImGui::End();
        return;
    }

    auto& padding = ImGui::GetStyle().FramePadding;
    auto& spacing = ImGui::GetStyle().ItemSpacing;
    auto font = ImGui::GetFont();

    auto draw_list = ImGui::GetWindowDrawList();

    // The text input widget occupies the bottom part
    // of the chat window, we need to reserve some space for it
    auto ypos = window_size.y - 2.5f * ImGui::GetFontSize() - 2.0f * padding.y - 2.0f * spacing.y;

    if(globals::gui_screen == GUI_CHAT) {
        if(needs_focus) {
            ImGui::SetKeyboardFocusHere();
            needs_focus = false;
        }

        ImGui::SetNextItemWidth(window_size.x + 32.0f * padding.x);
        ImGui::SetCursorScreenPos(ImVec2(padding.x, ypos));
        ImGui::InputText("###chat.input", &chat_input);
    }

    if(!client_game::hide_hud && ((globals::gui_screen == GUI_SCREEN_NONE) || (globals::gui_screen == GUI_CHAT))) {
        for(auto it = history.crbegin(); it < history.crend(); ++it) {
            auto text_size = ImGui::CalcTextSize(it->text.c_str(), it->text.c_str() + it->text.size(), false, window_size.x);
            auto rect_size = ImVec2(window_size.x, text_size.y + 2.0f * padding.y);

            auto rect_pos = ImVec2(padding.x, ypos - text_size.y - 2.0f * padding.y);
            auto rect_end = ImVec2(rect_pos.x + rect_size.x, rect_pos.y + rect_size.y);
            auto text_pos = ImVec2(rect_pos.x + padding.x, rect_pos.y + padding.y);

            auto fadeout_seconds = 10.0f;
            auto fadeout = std::exp(-1.0f * std::pow(1.0e-6 * static_cast<float>(globals::curtime - it->spawn) / fadeout_seconds, 10.0f));

            float rect_alpha;
            float text_alpha;

            if(globals::gui_screen == GUI_CHAT) {
                rect_alpha = 0.75f;
                text_alpha = 1.00f;
            }
            else {
                rect_alpha = 0.50f * fadeout;
                text_alpha = 1.00f * fadeout;
            }

            auto rect_col = ImGui::GetColorU32(ImGuiCol_FrameBg, rect_alpha);
            auto text_col = ImGui::GetColorU32(ImVec4(it->color.x, it->color.y, it->color.z, it->color.w * text_alpha));
            auto shadow_col = ImGui::GetColorU32(ImVec4(0.0f, 0.0f, 0.0f, text_alpha));

            draw_list->AddRectFilled(rect_pos, rect_end, rect_col);

            imdraw_ext::text_shadow_w(it->text, text_pos, text_col, shadow_col, font, draw_list, 8.0f, window_size.x);

            ypos -= rect_size.y;
        }
    }

    ImGui::End();
    ImGui::PopFont();
}

void gui::client_chat::clear(void)
{
    history.clear();
}

void gui::client_chat::refresh_timings(void)
{
    for(auto it = history.begin(); it < history.end(); ++it) {
        // Reset the spawn time so the fadeout timer
        // is reset; SpawnPlayer handler might call this
        it->spawn = globals::curtime;
    }
}

void gui::client_chat::print(const std::string& text)
{
    GuiChatMessage message = {};
    message.spawn = globals::curtime;
    message.text = text;
    message.color = ImGui::GetStyleColorVec4(ImGuiCol_Text);
    history.push_back(message);

    if(sfx_chat_message && session::is_ingame()) {
        sound::play_ui(sfx_chat_message, false, 1.0f);
    }
}
