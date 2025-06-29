#include "client/pch.hh"

#include "client/play_menu.hh"

#include "core/config.hh"
#include "core/constexpr.hh"
#include "core/strtools.hh"

#include "shared/protocol.hh"

#include "client/bother.hh"
#include "client/game.hh"
#include "client/glfw.hh"
#include "client/globals.hh"
#include "client/gui_screen.hh"
#include "client/language.hh"
#include "client/session.hh"

constexpr static ImGuiWindowFlags WINDOW_FLAGS = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration;
constexpr static const char* DEFAULT_SERVER_NAME = "Voxelius Server";
constexpr static const char* SERVERS_TXT = "servers.txt";
constexpr static const char* WARNING_TOAST = "[!]";

constexpr static std::size_t MAX_SERVER_ITEM_NAME = 24;

enum class item_status : unsigned int {
    UNKNOWN = 0x0000U,
    PINGING = 0x0001U,
    REACHED = 0x0002U,
    FAILURE = 0x0003U,
};

struct ServerStatusItem final {
    std::string name;
    std::string password;
    std::string hostname;
    std::uint16_t port;

    // Things pulled from bother events
    std::uint32_t protocol_version;
    std::uint16_t num_players;
    std::uint16_t max_players;
    std::string motd;

    // Unique identifier that monotonically
    // grows with each new server added and
    // doesn't reset with each server removed
    unsigned int identity;

    item_status status;
};

static std::string str_tab_servers;

static std::string str_join;
static std::string str_connect;
static std::string str_add;
static std::string str_edit;
static std::string str_remove;
static std::string str_refresh;

static std::string str_status_init;
static std::string str_status_ping;
static std::string str_status_fail;

static std::string str_outdated_client;
static std::string str_outdated_server;

static std::string input_itemname;
static std::string input_hostname;
static std::string input_password;

static unsigned int next_identity;
static std::deque<ServerStatusItem*> servers_deque;
static ServerStatusItem* selected_server;
static bool editing_server;
static bool adding_server;
static bool needs_focus;

static void parse_hostname(ServerStatusItem* item, const std::string& hostname)
{
    auto parts = strtools::split(hostname, ":");

    if(!parts[0].empty()) {
        item->hostname = parts[0];
    } else {
        item->hostname = std::string("localhost");
    }

    if(parts.size() >= 2) {
        item->port = vx::clamp<std::uint16_t>(strtoul(parts[1].c_str(), nullptr, 10), 1024, UINT16_MAX);
    } else {
        item->port = protocol::PORT;
    }
}

static void add_new_server(void)
{
    auto item = new ServerStatusItem();
    item->port = protocol::PORT;
    item->protocol_version = protocol::VERSION;
    item->max_players = UINT16_MAX;
    item->num_players = UINT16_MAX;
    item->identity = next_identity;
    item->status = item_status::UNKNOWN;

    next_identity += 1U;

    input_itemname = DEFAULT_SERVER_NAME;
    input_hostname = std::string();
    input_password = std::string();

    servers_deque.push_back(item);
    selected_server = item;
    editing_server = true;
    adding_server = true;
    needs_focus = true;
}

static void edit_selected_server(void)
{
    input_itemname = selected_server->name;

    if(selected_server->port != protocol::PORT) {
        input_hostname = std::format("{}:{}", selected_server->hostname, selected_server->port);
    } else {
        input_hostname = selected_server->hostname;
    }

    input_password = selected_server->password;

    editing_server = true;
    needs_focus = true;
}

static void remove_selected_server(void)
{
    bother::cancel(selected_server->identity);

    for(auto it = servers_deque.cbegin(); it != servers_deque.cend(); ++it) {
        if(selected_server == (*it)) {
            delete selected_server;
            selected_server = nullptr;
            servers_deque.erase(it);
            return;
        }
    }
}

static void join_selected_server(void)
{
    if(!session::peer) {
        session::connect(selected_server->hostname.c_str(), selected_server->port, selected_server->password.c_str());
    }
}

static void on_glfw_key(const GlfwKeyEvent& event)
{
    if((event.key == GLFW_KEY_ESCAPE) && (event.action == GLFW_PRESS)) {
        if(globals::gui_screen == GUI_PLAY_MENU) {
            if(editing_server) {
                if(adding_server) {
                    remove_selected_server();
                } else {
                    input_itemname.clear();
                    input_hostname.clear();
                    input_password.clear();
                    editing_server = false;
                    adding_server = false;
                    return;
                }
            }

            globals::gui_screen = GUI_MAIN_MENU;
            selected_server = nullptr;
            return;
        }
    }
}

static void on_language_set(const LanguageSetEvent& event)
{
    str_tab_servers = language::resolve_gui("play_menu.tab.servers");

    str_join = language::resolve_gui("play_menu.join");
    str_connect = language::resolve_gui("play_menu.connect");
    str_add = language::resolve_gui("play_menu.add");
    str_edit = language::resolve_gui("play_menu.edit");
    str_remove = language::resolve_gui("play_menu.remove");
    str_refresh = language::resolve_gui("play_menu.refresh");

    str_status_init = language::resolve("play_menu.status.init");
    str_status_ping = language::resolve("play_menu.status.ping");
    str_status_fail = language::resolve("play_menu.status.fail");

    str_outdated_client = language::resolve("play_menu.outdated_client");
    str_outdated_server = language::resolve("play_menu.outdated_server");
}

static void on_bother_response(const BotherResponseEvent& event)
{
    for(auto item : servers_deque) {
        if(item->identity == event.identity) {
            if(event.is_server_unreachable) {
                item->protocol_version = 0U;
                item->num_players = UINT16_MAX;
                item->max_players = UINT16_MAX;
                item->motd = str_status_fail;
                item->status = item_status::FAILURE;
            } else {
                item->protocol_version = event.protocol_version;
                item->num_players = event.num_players;
                item->max_players = event.max_players;
                item->motd = event.motd;
                item->status = item_status::REACHED;
            }

            break;
        }
    }
}

static void layout_server_item(ServerStatusItem* item)
{
    // Preserve the cursor at which we draw stuff
    const ImVec2& cursor = ImGui::GetCursorScreenPos();
    const ImVec2& padding = ImGui::GetStyle().FramePadding;
    const ImVec2& spacing = ImGui::GetStyle().ItemSpacing;

    const float item_width = ImGui::GetContentRegionAvail().x;
    const float line_height = ImGui::GetTextLineHeightWithSpacing();
    const std::string sid = std::format("###play_menu.servers.{}", static_cast<void*>(item));
    if(ImGui::Selectable(sid.c_str(), (item == selected_server), 0, ImVec2(0.0, 2.0f * (line_height + padding.y + spacing.y)))) {
        selected_server = item;
        editing_server = false;
    }

    if(ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
        // Double clicked - join the selected server
        join_selected_server();
    }

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    if(item == selected_server) {
        const ImVec2 start = ImVec2(cursor.x, cursor.y);
        const ImVec2 end = ImVec2(start.x + item_width, start.y + 2.0f * (line_height + padding.y + spacing.y));
        draw_list->AddRect(start, end, ImGui::GetColorU32(ImGuiCol_Text), 0.0f, 0, globals::gui_scale);
    }

    const ImVec2 name_pos = ImVec2(cursor.x + padding.x + 0.5f * spacing.x, cursor.y + padding.y);
    draw_list->AddText(name_pos, ImGui::GetColorU32(ImGuiCol_Text), item->name.c_str(), item->name.c_str() + item->name.size());

    if(item->status == item_status::REACHED) {
        auto stats = std::format("{}/{}", item->num_players, item->max_players);
        auto stats_width = ImGui::CalcTextSize(stats.c_str(), stats.c_str() + stats.size()).x;
        auto stats_pos = ImVec2(cursor.x + item_width - stats_width - padding.x, cursor.y + padding.y);
        draw_list->AddText(stats_pos, ImGui::GetColorU32(ImGuiCol_TextDisabled), stats.c_str(), stats.c_str() + stats.size());

        if(item->protocol_version != protocol::VERSION) {
            auto warning_size = ImGui::CalcTextSize(WARNING_TOAST);
            auto warning_pos = ImVec2(stats_pos.x - warning_size.x - padding.x - 4.0f * globals::gui_scale, cursor.y + padding.y);
            auto warning_end = ImVec2(warning_pos.x + warning_size.x, warning_pos.y + warning_size.y);
            draw_list->AddText(warning_pos, ImGui::GetColorU32(ImGuiCol_DragDropTarget), WARNING_TOAST);

            if(ImGui::IsMouseHoveringRect(warning_pos, warning_end)) {
                ImGui::BeginTooltip();

                if(item->protocol_version < protocol::VERSION) {
                    ImGui::TextUnformatted(str_outdated_server.c_str(), str_outdated_server.c_str() + str_outdated_server.size());
                } else {
                    ImGui::TextUnformatted(str_outdated_client.c_str(), str_outdated_client.c_str() + str_outdated_client.size());
                }

                ImGui::EndTooltip();
            }
        }
    }

    ImU32 motd_color = {};
    const std::string* motd_text;

    switch(item->status) {
        case item_status::UNKNOWN:
            motd_color = ImGui::GetColorU32(ImGuiCol_TextDisabled);
            motd_text = &str_status_init;
            break;
        case item_status::PINGING:
            motd_color = ImGui::GetColorU32(ImGuiCol_TextDisabled);
            motd_text = &str_status_ping;
            break;
        case item_status::REACHED:
            motd_color = ImGui::GetColorU32(ImGuiCol_TextDisabled);
            motd_text = &item->motd;
            break;
        default:
            motd_color = ImGui::GetColorU32(ImGuiCol_PlotLinesHovered);
            motd_text = &str_status_fail;
            break;
    }

    const ImVec2 motd_pos = ImVec2(cursor.x + padding.x + 0.5f * spacing.x, cursor.y + padding.y + line_height);
    draw_list->AddText(motd_pos, motd_color, motd_text->c_str(), motd_text->c_str() + motd_text->size());
}

static void layout_server_edit(ServerStatusItem* item)
{
    if(needs_focus) {
        ImGui::SetKeyboardFocusHere();
        needs_focus = false;
    }

    ImGui::SetNextItemWidth(-0.25f * ImGui::GetContentRegionAvail().x);
    ImGui::InputText("###play_menu.servers.edit_itemname", &input_itemname);
    ImGui::SameLine();

    const bool ignore_input = strtools::is_whitespace(input_itemname) || input_hostname.empty();

    ImGui::BeginDisabled(ignore_input);

    if(ImGui::Button("OK###play_menu.servers.submit_input", ImVec2(-1.0f, 0.0f))
        || (!ignore_input && ImGui::IsKeyPressed(ImGuiKey_Enter))) {
        parse_hostname(item, input_hostname);
        item->password = input_password;
        item->name = input_itemname.substr(0, MAX_SERVER_ITEM_NAME);
        item->status = item_status::UNKNOWN;
        editing_server = false;
        adding_server = false;

        input_itemname.clear();
        input_hostname.clear();

        bother::cancel(item->identity);
    }

    ImGui::EndDisabled();

    ImGuiInputTextFlags hostname_flags = ImGuiInputTextFlags_CharsNoBlank;

    if(client_game::streamer_mode.get_value()) {
        // Hide server hostname to avoid things like
        // followers flooding the server that is streamed online
        hostname_flags |= ImGuiInputTextFlags_Password;
    }

    ImGui::SetNextItemWidth(-0.50f * ImGui::GetContentRegionAvail().x);
    ImGui::InputText("###play_menu.servers.edit_hostname", &input_hostname, hostname_flags);
    ImGui::SameLine();

    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    ImGui::InputText("###play_menu.servers.edit_password", &input_password, ImGuiInputTextFlags_Password);
}

static void layout_servers(void)
{
    if(ImGui::BeginListBox("###play_menu.servers.listbox", ImVec2(-1.0f, -1.0f))) {
        for(ServerStatusItem* item : servers_deque) {
            if(editing_server && item == selected_server) {
                layout_server_edit(item);
            } else {
                layout_server_item(item);
            }
        }

        ImGui::EndListBox();
    }
}

static void layout_servers_buttons(void)
{
    auto avail_width = ImGui::GetContentRegionAvail().x;

    // Can only join when selected and not editing
    ImGui::BeginDisabled(!selected_server || editing_server);

    if(ImGui::Button(str_join.c_str(), ImVec2(-0.50f * avail_width, 0.0f))) {
        join_selected_server();
    }

    ImGui::EndDisabled();
    ImGui::SameLine();

    // Can only connect directly when not editing anything
    ImGui::BeginDisabled(editing_server);

    if(ImGui::Button(str_connect.c_str(), ImVec2(-1.00f, 0.0f))) {
        globals::gui_screen = GUI_DIRECT_CONNECTION;
    }

    ImGui::EndDisabled();

    // Can only add when not editing anything
    ImGui::BeginDisabled(editing_server);

    if(ImGui::Button(str_add.c_str(), ImVec2(-0.75f * avail_width, 0.0f))) {
        add_new_server();
    }

    ImGui::EndDisabled();
    ImGui::SameLine();

    // Can only edit when selected and not editing
    ImGui::BeginDisabled(!selected_server || editing_server);

    if(ImGui::Button(str_edit.c_str(), ImVec2(-0.50f * avail_width, 0.0f))) {
        edit_selected_server();
    }

    ImGui::EndDisabled();
    ImGui::SameLine();

    // Can only remove when selected and not editing
    ImGui::BeginDisabled(!selected_server || editing_server);

    if(ImGui::Button(str_remove.c_str(), ImVec2(-0.25f * avail_width, 0.0f))) {
        remove_selected_server();
    }

    ImGui::EndDisabled();
    ImGui::SameLine();

    if(ImGui::Button(str_refresh.c_str(), ImVec2(-1.0f, 0.0f))) {
        for(ServerStatusItem* item : servers_deque) {
            if(item->status != item_status::PINGING) {
                if(!editing_server || item != selected_server) {
                    item->status = item_status::UNKNOWN;
                    bother::cancel(item->identity);
                }
            }
        }
    }
}

void play_menu::init(void)
{
    if(auto file = PHYSFS_openRead(SERVERS_TXT)) {
        auto source = std::string(PHYSFS_fileLength(file), char(0x00));
        PHYSFS_readBytes(file, source.data(), source.size());
        PHYSFS_close(file);

        auto stream = std::istringstream(source);
        auto line = std::string();

        while(std::getline(stream, line)) {
            auto parts = strtools::split(line, "%");

            auto item = new ServerStatusItem();
            item->port = protocol::PORT;
            item->protocol_version = protocol::VERSION;
            item->max_players = UINT16_MAX;
            item->num_players = UINT16_MAX;
            item->identity = next_identity;
            item->status = item_status::UNKNOWN;

            next_identity += 1U;

            parse_hostname(item, parts[0]);

            if(parts.size() >= 2) {
                item->password = parts[1];
            } else {
                item->password = std::string();
            }

            if(parts.size() >= 3) {
                item->name = parts[2].substr(0, MAX_SERVER_ITEM_NAME);
            } else {
                item->name = DEFAULT_SERVER_NAME;
            }

            servers_deque.push_back(item);
        }
    }

    globals::dispatcher.sink<GlfwKeyEvent>().connect<&on_glfw_key>();
    globals::dispatcher.sink<LanguageSetEvent>().connect<&on_language_set>();
    globals::dispatcher.sink<BotherResponseEvent>().connect<&on_bother_response>();
}

void play_menu::shutdown(void)
{
    std::ostringstream stream;

    for(const auto item : servers_deque) {
        stream << std::format("{}:{}%{}%{}", item->hostname, item->port, item->password, item->name) << std::endl;
    }

    if(auto file = PHYSFS_openWrite(SERVERS_TXT)) {
        auto source = stream.str();
        PHYSFS_writeBytes(file, source.data(), source.size());
        PHYSFS_close(file);
    }

    for(auto item : servers_deque)
        delete item;
    servers_deque.clear();
}

void play_menu::layout(void)
{
    const auto viewport = ImGui::GetMainViewport();
    const auto window_start = ImVec2(viewport->Size.x * 0.05f, viewport->Size.y * 0.05f);
    const auto window_size = ImVec2(viewport->Size.x * 0.90f, viewport->Size.y * 0.90f);

    ImGui::SetNextWindowPos(window_start);
    ImGui::SetNextWindowSize(window_size);

    if(ImGui::Begin("###play_menu", nullptr, WINDOW_FLAGS)) {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(3.0f * globals::gui_scale, 3.0f * globals::gui_scale));

        if(ImGui::BeginTabBar("###play_menu.tabs", ImGuiTabBarFlags_FittingPolicyResizeDown)) {
            if(ImGui::TabItemButton("<<")) {
                globals::gui_screen = GUI_MAIN_MENU;
                selected_server = nullptr;
                editing_server = false;
            }

            if(ImGui::BeginTabItem(str_tab_servers.c_str())) {
                if(ImGui::BeginChild("###play_menu.servers.child", ImVec2(0.0f, -2.0f * ImGui::GetFrameHeightWithSpacing()))) {
                    layout_servers();
                }

                ImGui::EndChild();

                layout_servers_buttons();

                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::PopStyleVar();
    }

    ImGui::End();
}

void play_menu::update_late(void)
{
    for(auto item : servers_deque) {
        if(item->status == item_status::UNKNOWN) {
            bother::ping(item->identity, item->hostname.c_str(), item->port);
            item->status = item_status::PINGING;
            continue;
        }
    }
}
