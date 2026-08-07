#include "client/pch.hh"

#include "client/play_menu.hh"

#include "core/exception.hh"
#include "core/utils/physfs.hh"
#include "core/utils/string.hh"
#include "core/version.hh"

#include "shared/net/packet_session.hh"

#include "client/fonts.hh"
#include "client/globals.hh"
#include "client/gui/background.hh"
#include "client/gui/button.hh"
#include "client/gui/container.hh"
#include "client/gui/dimmer.hh"
#include "client/gui/label.hh"
#include "client/gui/list_box.hh"
#include "client/gui/panel.hh"
#include "client/gui/popup.hh"
#include "client/gui/screen.hh"
#include "client/gui/stack.hh"
#include "client/gui/tabs.hh"
#include "client/language.hh"
#include "client/main_menu.hh"
#include "client/net/bother.hh"
#include "client/net/session.hh"

constexpr static float ROW_HEIGHT = 24.0f;
constexpr static float BODY_MARGIN = 32.0f;
constexpr static std::uint16_t DEFAULT_PORT = 16384;
constexpr static std::string_view DEFAULT_NAME = "Voxelius Server";
constexpr static std::string_view JSON_PATH = "servers.json";

class ServerListItem final : public gui::ListBoxItemBuilder<ServerListItem> {
public:
    constexpr static unsigned UNKNOWN = 0;
    constexpr static unsigned PINGING = 1;
    constexpr static unsigned REACHED = 2;
    constexpr static unsigned UNREACHABLE = 3;

    virtual void layout(void) override;

    void invalidate(void);

    unsigned status { UNKNOWN };
    unsigned identity;
    bool configured { false };

    std::string name;
    std::string host;
    std::uint16_t port;
    std::uint64_t invite;

    std::uint16_t num_players { 0 };
    std::uint16_t max_players { 0 };
    std::uint32_t version_major { 0 };
    std::uint32_t version_minor { 0 };
    std::uint32_t version_patch { 0 };
    std::string motd;

    std::optional<std::string> stats_string {};
    std::optional<std::string> version_string {};
};

static std::string s_motd_init {};
static std::string s_motd_ping {};
static std::string s_motd_fail {};

static std::deque<std::unique_ptr<ServerListItem>> s_servers;
static emhash8::HashMap<unsigned, ServerListItem*> s_server_map;
static unsigned s_next_identity;

static gui::ListBox s_server_list;
static ServerListItem* s_selected_server;

static gui::Button s_btn_join;
static gui::Button s_btn_connect;
static gui::Button s_btn_add;
static gui::Button s_btn_edit;
static gui::Button s_btn_remove;
static gui::Button s_btn_refresh;
static gui::HorizontalStack s_servers_buttons_row1;
static gui::HorizontalStack s_servers_buttons_row2;

static gui::Dimmer s_dimmer;
static gui::Panel s_body;
static gui::Tabs s_tabs;

static gui::Label s_worlds_todo;
static gui::Container s_worlds_tab;
static gui::VerticalStack s_servers_tab;
static gui::InputPopup s_server_popup;
static gui::InputPopup s_direct_popup;
static gui::ProgressPopup s_connect_popup;
static gui::ChoicePopup s_error_popup;

gui::Screen play_menu::screen;

void ServerListItem::layout(void)
{
    const auto& style = ImGui::GetStyle();
    const auto& padding = style.FramePadding;
    const auto& spacing = style.ItemSpacing;

    auto item_size = ImGui::GetContentRegionAvail();
    auto line_height = ImGui::GetTextLineHeightWithSpacing();
    auto row_height = 2.0f * (line_height + padding.y + spacing.y);

    auto cursor_pos = this->begin_row(ImVec2(0.0f, row_height));
    auto draw_list = ImGui::GetWindowDrawList();

    ImVec2 name_pos {};
    name_pos.x = cursor_pos.x + padding.x + 0.5f * spacing.x;
    name_pos.y = cursor_pos.y + padding.y;

    draw_list->AddText(name_pos, ImGui::GetColorU32(ImGuiCol_Text), name.c_str(), name.c_str() + name.size());

    if(status == REACHED) {
        if(!stats_string.has_value()) {
            stats_string = std::format("{}/{}", num_players, max_players);
        }

        auto stats_cbegin = stats_string->data();
        auto stats_cend = stats_cbegin + stats_string->size();
        auto stats_size = ImGui::CalcTextSize(stats_cbegin, stats_cend);

        ImVec2 stats_pos {};
        stats_pos.x = cursor_pos.x + item_size.x - stats_size.x - padding.x - 0.5f * spacing.x;
        stats_pos.y = cursor_pos.y + padding.y;

        draw_list->AddText(stats_pos, ImGui::GetColorU32(ImGuiCol_TextDisabled), stats_cbegin, stats_cend);

        auto major_match = version_major == version::major;
        auto minor_match = version_minor == version::minor;
        auto patch_match = version_patch == version::patch;

        ImU32 version_color {};

        if(major_match && minor_match && patch_match) {
            version_color = ImGui::GetColorU32(ImGuiCol_PlotHistogram);
        }
        else if(major_match && minor_match) {
            version_color = ImGui::GetColorU32(ImGuiCol_DragDropTarget);
        }
        else {
            version_color = ImGui::GetColorU32(ImGuiCol_PlotLinesHovered);
        }

        if(!version_string.has_value()) {
            // NOTE: we're not doing the 16.x.x thing
            // and not bothering with custom diplay formats
            // now since 16.x.x and 17.x.x are fundamentally different
            version_string = std::format("{}.{}.{}", version_major, version_minor, version_patch);
        }

        ImGui::PushFont(fonts::unscii8, 8.0f);

        auto version_cbegin = version_string->data();
        auto version_cend = version_cbegin + version_string->size();
        auto version_size = ImGui::CalcTextSize(version_cbegin, version_cend);

        ImVec2 version_pos {};
        version_pos.x = stats_pos.x - version_size.x - padding.x - 0.5f * spacing.x;
        version_pos.y = stats_pos.y + 0.5f * (stats_size.y - version_size.y);

        ImVec2 version_end {};
        version_end.x = version_pos.x + version_size.x;
        version_end.y = version_pos.y + version_size.y;

        ImVec2 outline_pos {};
        outline_pos.x = version_pos.x - 2.0f * static_cast<float>(globals::gui_scale);
        outline_pos.y = std::max(cursor_pos.y, version_pos.y - 2.0f * static_cast<float>(globals::gui_scale));

        ImVec2 outline_end {};
        outline_end.x = version_end.x + 2.0f * static_cast<float>(globals::gui_scale);
        outline_end.y = version_end.y + 2.0f * static_cast<float>(globals::gui_scale);

        auto outline_thickness = std::max(1.0f, 0.5f * static_cast<float>(globals::gui_scale));

        draw_list->AddRect(outline_pos, outline_end, version_color, 0.0f, 0, outline_thickness);
        draw_list->AddText(version_pos, version_color, version_cbegin, version_cend);

        ImGui::PopFont();
    }

    ImU32 motd_color {};
    std::string_view motd_text {};

    switch(status) {
        case UNKNOWN:
            motd_color = ImGui::GetColorU32(ImGuiCol_TextDisabled);
            motd_text = s_motd_init;
            break;

        case PINGING:
            motd_color = ImGui::GetColorU32(ImGuiCol_TextDisabled);
            motd_text = s_motd_ping;
            break;

        case REACHED:
            motd_color = ImGui::GetColorU32(ImGuiCol_TextDisabled);
            motd_text = motd;
            break;

        default:
            motd_color = ImGui::GetColorU32(ImGuiCol_PlotLinesHovered);
            motd_text = s_motd_fail;
            break;
    }

    auto motd_cbegin = motd_text.data();
    auto motd_cend = motd_cbegin + motd_text.size();

    ImVec2 motd_pos {};
    motd_pos.x = cursor_pos.x + padding.x + 0.5f * spacing.x;
    motd_pos.y = cursor_pos.y + padding.y + line_height;

    draw_list->AddText(motd_pos, motd_color, motd_cbegin, motd_cend);
}

void ServerListItem::invalidate(void)
{
    stats_string.reset();
    version_string.reset();
}

static void select_server(ServerListItem* item)
{
    if(s_selected_server) {
        s_selected_server->set_selected(false);
    }

    s_selected_server = item;

    if(s_selected_server) {
        s_selected_server->set_selected(true);
    }

    s_btn_join.set_enabled(s_selected_server != nullptr);
    s_btn_edit.set_enabled(s_selected_server != nullptr);
    s_btn_remove.set_enabled(s_selected_server != nullptr);
}

static void join_server(ServerListItem* item)
{
    session::connect(item->host, item->port, item->invite);

    item->invite = 0; // clear invite after joining; FIXME: should we do that after a successful join instead?
}

static ServerListItem* create_server(void)
{
    auto owned = std::make_unique<ServerListItem>();
    auto item = owned.get();

    item->identity = s_next_identity;
    s_next_identity += 1;

    item->on_single_click([item] {
        select_server(item);
    });

    item->on_double_click([item] {
        join_server(item);
    });

    s_server_map.emplace(item->identity, item);
    s_server_list.add_child(*item);
    s_servers.push_back(std::move(owned));

    return item;
}

static void destroy_server(ServerListItem* item)
{
    if(s_selected_server == item) {
        select_server(nullptr);
    }

    bother::cancel(item->identity);

    s_server_list.remove_child(*item);
    s_server_map.erase(item->identity);

    std::erase_if(s_servers, [item](const std::unique_ptr<ServerListItem>& owned) {
        return item == owned.get();
    });
}

static std::pair<std::string_view, std::uint16_t> parse_hostname(std::string_view hostname)
{
    auto colon = hostname.find(':');

    if(colon == std::string_view::npos) {
        if(hostname.empty()) {
            return std::make_pair(std::string_view("localhost"), DEFAULT_PORT);
        }
        else {
            return std::make_pair(hostname, DEFAULT_PORT);
        }
    }
    else {
        auto host_part = hostname.substr(0, colon);
        auto port_part = hostname.substr(colon + 1);

        std::uint16_t port;
        auto check = std::from_chars(port_part.data(), port_part.data() + port_part.size(), port);

        if(check.ec == std::errc {}) {
            return std::make_pair(host_part, std::max<std::uint16_t>(1024, port));
        }
        else {
            return std::make_pair(host_part, DEFAULT_PORT);
        }
    }
}

static void apply_popup_values(ServerListItem* item, std::span<const std::string> values)
{
    auto parts = parse_hostname(values[1]);
    auto name = values[0].substr(0, 18);

    if(utils::is_whitespace<char>(name)) {
        item->name = DEFAULT_NAME;
    }
    else {
        item->name = name;
    }

    item->host = parts.first;
    item->port = parts.second;
    item->invite = 0;
    item->status = ServerListItem::UNKNOWN;
    item->configured = true;

    if(values[2].size()) {
        std::uint64_t invite;
        auto check = std::from_chars(values[2].data(), values[2].data() + values[2].size(), invite);

        if(check.ec == std::errc {}) {
            item->invite = invite;
        }
    }

    item->invalidate();
}

static void open_add_popup(void)
{
    auto item = create_server();
    item->port = DEFAULT_PORT;

    s_server_popup.set_value(0, DEFAULT_NAME);
    s_server_popup.set_value(1, {});
    s_server_popup.set_value(2, {});

    s_server_popup.on_submit([item](std::span<const std::string> values) {
        apply_popup_values(item, values);
    });

    s_server_popup.on_cancel([item] {
        destroy_server(item);
    });

    s_server_popup.open();

    select_server(item);
}

static void open_edit_popup(void)
{
    if(s_selected_server) {
        auto item = s_selected_server;
        std::string hostname;

        if(item->port == DEFAULT_PORT) {
            hostname = item->host;
        }
        else {
            hostname = std::format("{}:{}", item->host, item->port);
        }

        s_server_popup.set_value(0, item->name);
        s_server_popup.set_value(1, hostname);

        if(item->invite) {
            s_server_popup.set_value(2, std::to_string(item->invite));
        }
        else {
            s_server_popup.set_value(2, {});
        }

        s_server_popup.on_submit([item](std::span<const std::string> values) {
            apply_popup_values(item, values);
        });

        s_server_popup.on_cancel({});

        s_server_popup.open();
    }
}

static void refresh_servers(void)
{
    for(auto& owned : s_servers) {
        bother::cancel(owned->identity);
        owned->status = ServerListItem::UNKNOWN;
        owned->invalidate();
    }
}

static void on_bother_response(const BotherResponseEvent& event)
{
    auto it = s_server_map.find(event.request_id());

    if(it == s_server_map.end()) {
        return;
    }

    auto item = it->second;

    if(event.unreachable()) {
        item->status = ServerListItem::UNREACHABLE;
    }
    else {
        item->status = ServerListItem::REACHED;
        item->num_players = event.num_players();
        item->max_players = event.max_players();
        item->version_major = event.version_major();
        item->version_minor = event.version_minor();
        item->version_patch = event.version_patch();
        item->motd = event.motd();
    }

    item->invalidate();
}

static void on_language_update(const LanguageUpdateEvent& event)
{
    play_menu::screen.translate();

    s_motd_init = language::resolve("play_menu.servers.motd_init");
    s_motd_ping = language::resolve("play_menu.servers.motd_ping");
    s_motd_fail = language::resolve("play_menu.servers.motd_fail");
}

static void on_keyboard_event(const SDL_KeyboardEvent& event)
{
    auto should_close = true;
    should_close = should_close && event.type == SDL_EVENT_KEY_DOWN && event.key == SDLK_ESCAPE;
    should_close = should_close && globals::gui_screen == &play_menu::screen;
    should_close = should_close && !ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopupId);

    if(should_close) {
        globals::gui_screen = &main_menu::screen;
    }
}

static void on_session_state(const SessionStateEvent& event)
{
    switch(event.state()) {
        case SESSION_CONNECTING:
            play_menu::open_connect_popup("play_menu.connect.stage.connecting", [] {
                session::disconnect(Disconnect_Packet::CLIENT_DISCONNECT);
            });
            break;

        case SESSION_AUTHENTICATING:
            play_menu::set_connect_message("play_menu.connect.stage.authenticating");
            break;

        case SESSION_SPAWNING:
            play_menu::set_connect_message("play_menu.connect.stage.spawning");
            break;

        case SESSION_INGAME:
            play_menu::close_connect_popup();
            globals::gui_screen = nullptr;
            break;

        case SESSION_DISCONNECTED:
            play_menu::close_connect_popup();
            play_menu::show_error(Disconnect_Packet::reason_string_client(event.reason()));
            break;
    }
}

static void load_servers_json(void)
{
    std::string source;

    if(utils::read_file(JSON_PATH, source)) {
        auto jsonv = json_parse_string(source.c_str());
        auto json = json_value_get_object(jsonv);

        if(jsonv == nullptr) {
            LOG_WARNING("{}: malformed JSON", JSON_PATH);
            return;
        }

        if(json == nullptr) {
            LOG_WARNING("{}: expected JSON object", JSON_PATH);
            json_value_free(jsonv);
            return;
        }

        auto direct_hostname = json_object_get_string(json, "direct_hostname");
        auto direct_invite_str = json_object_get_string(json, "direct_invite");
        auto servers = json_object_get_array(json, "servers");

        if(direct_hostname == nullptr || direct_invite_str == nullptr || servers == nullptr) {
            LOG_WARNING("{}: expected JSON object with direct_hostname, direct_invite, and servers keys", JSON_PATH);
            json_value_free(jsonv);
            return;
        }

        std::uint64_t direct_invite {};
        auto direct_invite_size = std::strlen(direct_invite_str);
        auto direct_invite_check = std::from_chars(direct_invite_str, direct_invite_str + direct_invite_size, direct_invite);

        if(direct_invite_check.ec == std::errc {}) {
            s_direct_popup.set_value(0, direct_hostname);
            s_direct_popup.set_value(1, std::to_string(direct_invite));
        }

        auto count = json_array_get_count(servers);

        for(std::size_t i = 0; i < count; ++i) {
            auto item = json_array_get_object(servers, i);
            auto name = json_object_get_string(item, "name");
            auto hostname = json_object_get_string(item, "hostname");
            auto invite_str = json_object_get_string(item, "invite");

            if(item == nullptr || name == nullptr || hostname == nullptr || invite_str == nullptr) {
                LOG_WARNING("{}: expected JSON object with name, hostname, and invite keys", JSON_PATH);
                continue;
            }

            std::uint64_t invite {};
            auto invite_size = std::strlen(invite_str);
            auto invite_check = std::from_chars(invite_str, invite_str + invite_size, invite);

            if(invite_check.ec == std::errc {}) {
                auto parts = parse_hostname(hostname);
                auto server = create_server();
                server->name = name;
                server->host = parts.first;
                server->port = parts.second;
                server->invite = invite;
                server->status = ServerListItem::UNKNOWN;
                server->configured = true;
                server->invalidate();
            }
        }

        json_value_free(jsonv);
    }
}

static void save_servers_json(void)
{
    auto jsonv = json_value_init_object();
    auto json = json_value_get_object(jsonv);
    vx::throw_if(jsonv == nullptr || json == nullptr);

    json_object_set_string(json, "direct_hostname", std::string(s_direct_popup.value(0)).c_str());
    json_object_set_string(json, "direct_invite", std::string(s_direct_popup.value(1)).c_str());

    auto serversv = json_value_init_array();
    auto servers = json_value_get_array(serversv);
    vx::throw_if(serversv == nullptr || servers == nullptr);

    for(auto& owned : s_servers) {
        if(owned->configured) {
            auto itemv = json_value_init_object();
            auto item = json_value_get_object(itemv);
            vx::throw_if(itemv == nullptr || item == nullptr);

            auto hostname = std::format("{}:{}", owned->host, owned->port);
            auto invite_str = std::to_string(owned->invite);

            json_object_set_string(item, "name", owned->name.c_str());
            json_object_set_string(item, "hostname", hostname.c_str());
            json_object_set_string(item, "invite", invite_str.c_str());

            json_array_append_value(servers, itemv);
        }
    }

    json_object_set_value(json, "servers", serversv);

    std::string source;
    source.resize(json_serialization_size(jsonv));
    json_serialize_to_buffer(jsonv, source.data(), source.size());

    utils::write_file(JSON_PATH, source);

    json_value_free(jsonv);
}

void play_menu::init(void)
{
    s_next_identity = 0;

    screen.add_child(main_menu::background, 0);
    screen.add_child(s_dimmer, 1);
    screen.add_child(s_body, 2);

    s_body.set_margin(ImVec2(BODY_MARGIN, BODY_MARGIN));

    s_worlds_todo.set_text("play_menu.worlds.todo");
    s_worlds_tab.add_child(s_worlds_todo, 0);

    s_btn_join.set_text("play_menu.servers.join");
    s_btn_join.set_size(-1.0f, -1.0f);
    s_btn_join.set_enabled(false);
    s_btn_join.on_click([] {
        if(s_selected_server) {
            join_server(s_selected_server);
        }
    });

    s_btn_connect.set_text("play_menu.servers.connect");
    s_btn_connect.set_size(-1.0f, -1.0f);
    s_btn_connect.on_click([] {
        s_direct_popup.open();
    });

    s_btn_add.set_text("play_menu.servers.add");
    s_btn_add.set_size(-1.0f, -1.0f);
    s_btn_add.on_click(&open_add_popup);

    s_btn_edit.set_text("play_menu.servers.edit");
    s_btn_edit.set_size(-1.0f, -1.0f);
    s_btn_edit.set_enabled(false);
    s_btn_edit.on_click(&open_edit_popup);

    s_btn_remove.set_text("play_menu.servers.remove");
    s_btn_remove.set_size(-1.0f, -1.0f);
    s_btn_remove.set_enabled(false);
    s_btn_remove.on_click([] {
        if(s_selected_server) {
            destroy_server(s_selected_server);
        }
    });

    s_btn_refresh.set_text("play_menu.servers.refresh");
    s_btn_refresh.set_size(-1.0f, -1.0f);
    s_btn_refresh.on_click(&refresh_servers);

    s_servers_buttons_row1.add_item(s_btn_join, gui::EXPANDING);
    s_servers_buttons_row1.add_item(s_btn_connect, gui::EXPANDING);

    s_servers_buttons_row2.add_item(s_btn_add, gui::EXPANDING);
    s_servers_buttons_row2.add_item(s_btn_edit, gui::EXPANDING);
    s_servers_buttons_row2.add_item(s_btn_remove, gui::EXPANDING);
    s_servers_buttons_row2.add_item(s_btn_refresh, gui::EXPANDING);

    s_servers_tab.add_item(s_server_list, gui::EXPANDING);
    s_servers_tab.add_item(s_servers_buttons_row1, gui::FIXED, ROW_HEIGHT);
    s_servers_tab.add_item(s_servers_buttons_row2, gui::FIXED, ROW_HEIGHT);

    s_tabs.add_tab("play_menu.tab.worlds", s_worlds_tab, 0);
    s_tabs.add_tab("play_menu.tab.servers", s_servers_tab, 1);
    s_tabs.set_bar_height(ROW_HEIGHT);
    s_tabs.set_callback([] {
        globals::gui_screen = &main_menu::screen;
    });

    s_body.add_child(s_tabs);

    s_server_popup.set_title("play_menu.servers.popup.title");
    s_server_popup.add_input("play_menu.servers.popup.name", ImGuiInputTextFlags_None);
    s_server_popup.add_input("play_menu.servers.popup.hostname", ImGuiInputTextFlags_CharsNoBlank);
    s_server_popup.add_input("play_menu.servers.popup.invite", ImGuiInputTextFlags_CharsDecimal);
    screen.add_child(s_server_popup, 3);

    s_direct_popup.set_title("play_menu.servers.direct.title");
    s_direct_popup.add_input("play_menu.servers.direct.hostname", ImGuiInputTextFlags_CharsNoBlank);
    s_direct_popup.add_input("play_menu.servers.direct.invite", ImGuiInputTextFlags_CharsDecimal);
    screen.add_child(s_direct_popup, 3);

    s_direct_popup.on_submit([](std::span<const std::string> values) {
        auto parts = parse_hostname(values[0]);

        std::uint64_t invite = 0;
        auto invite_str = values[1];
        auto invite_size = invite_str.size();

        if(invite_size) {
            std::from_chars(invite_str.data(), invite_str.data() + invite_size, invite);
        }

        session::connect(parts.first, parts.second, invite);
    });

    s_connect_popup.set_title("play_menu.connect.title");
    s_connect_popup.set_min_size(220.0f, 0.0f);
    screen.add_child(s_connect_popup, 3);

    s_error_popup.set_title("play_menu.connect.error.title");
    s_error_popup.add_choice("play_menu.connect.error.ok");
    screen.add_child(s_error_popup, 3);

    globals::dispatcher.sink<LanguageUpdateEvent>().connect<&on_language_update>();
    globals::dispatcher.sink<SDL_KeyboardEvent>().connect<&on_keyboard_event>();
    globals::dispatcher.sink<BotherResponseEvent>().connect<&on_bother_response>();
    globals::dispatcher.sink<SessionStateEvent>().connect<&on_session_state>();

    load_servers_json();
}

void play_menu::shutdown(void)
{
    save_servers_json();

    s_servers.clear();
}

void play_menu::update_late(void)
{
    for(auto& owned : s_servers) {
        if(owned->configured && owned->status == ServerListItem::UNKNOWN) {
            bother::ping(owned->identity, owned->host, owned->port);
            owned->status = ServerListItem::PINGING;
            owned->invalidate();
        }
    }
}

void play_menu::open_connect_popup(std::string_view message, std::function<void(void)> on_cancel)
{
    s_connect_popup.set_message(message);
    s_connect_popup.set_progress(std::nullopt);
    s_connect_popup.on_cancel(std::move(on_cancel));
    s_connect_popup.open();
}

void play_menu::set_connect_message(std::string_view message)
{
    s_connect_popup.set_message(message);
}

void play_menu::set_connect_progress(std::optional<float> progress)
{
    s_connect_popup.set_progress(progress);
}

void play_menu::close_connect_popup(void)
{
    s_connect_popup.close();
}

void play_menu::show_error(std::string_view message)
{
    s_error_popup.set_message(message);
    s_error_popup.open();
}
