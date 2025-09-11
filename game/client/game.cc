#include "client/pch.hh"

#include "client/game.hh"

#include "core/config/boolean.hh"
#include "core/config/number.hh"
#include "core/config/string.hh"

#include "core/io/config_map.hh"

#include "core/math/angles.hh"

#include "core/resource/resource.hh"

#include "core/utils/physfs.hh"

#include "shared/entity/collision.hh"
#include "shared/entity/gravity.hh"
#include "shared/entity/head.hh"
#include "shared/entity/player.hh"
#include "shared/entity/stasis.hh"
#include "shared/entity/transform.hh"
#include "shared/entity/velocity.hh"

#include "shared/game_items.hh"
#include "shared/game_voxels.hh"

#include "shared/world/dimension.hh"
#include "shared/world/item_registry.hh"
#include "shared/world/ray_dda.hh"
#include "shared/world/voxel_registry.hh"

#include "shared/coord.hh"
#include "shared/protocol.hh"

#include "client/config/keybind.hh"

#include "client/entity/camera.hh"
#include "client/entity/interpolation.hh"
#include "client/entity/listener.hh"
#include "client/entity/player_look.hh"
#include "client/entity/player_move.hh"
#include "client/entity/sound_emitter.hh"

#include "client/gui/background.hh"
#include "client/gui/bother.hh"
#include "client/gui/chat.hh"
#include "client/gui/crosshair.hh"
#include "client/gui/direct_connection.hh"
#include "client/gui/gui_screen.hh"
#include "client/gui/hotbar.hh"
#include "client/gui/language.hh"
#include "client/gui/main_menu.hh"
#include "client/gui/message_box.hh"
#include "client/gui/metrics.hh"
#include "client/gui/play_menu.hh"
#include "client/gui/progress_bar.hh"
#include "client/gui/scoreboard.hh"
#include "client/gui/settings.hh"
#include "client/gui/splash.hh"
#include "client/gui/status_lines.hh"
#include "client/gui/window_title.hh"

#include "client/io/gamepad.hh"
#include "client/io/glfw.hh"

#include "client/resource/texture_gui.hh"

#include "client/sound/sound.hh"

#include "client/world/chunk_mesher.hh"
#include "client/world/chunk_renderer.hh"
#include "client/world/chunk_visibility.hh"
#include "client/world/outline.hh"
#include "client/world/player_target.hh"
#include "client/world/skybox.hh"
#include "client/world/voxel_anims.hh"
#include "client/world/voxel_atlas.hh"
#include "client/world/voxel_sounds.hh"

#include "client/const.hh"
#include "client/experiments.hh"
#include "client/globals.hh"
#include "client/receive.hh"
#include "client/screenshot.hh"
#include "client/session.hh"
#include "client/toggles.hh"

config::Boolean client_game::streamer_mode(false);
config::Boolean client_game::vertical_sync(true);
config::Boolean client_game::world_curvature(true);
config::Unsigned client_game::fog_mode(1U, 0U, 2U);
config::String client_game::username("player");

bool client_game::hide_hud = false;

static config::KeyBind hide_hud_toggle(GLFW_KEY_F1);

static ImFont* load_font(std::string_view path, float size, ImFontConfig& font_config, ImVector<ImWchar>& ranges)
{
    bool font_load_success;
    std::vector<std::byte> font;

    if(!utils::read_file(path, font)) {
        spdlog::error("{}: utils::read_file failed", path);
        std::terminate();
    }

    auto& io = ImGui::GetIO();
    auto font_ptr = io.Fonts->AddFontFromMemoryTTF(font.data(), font.size(), size, &font_config, ranges.Data);

    if(font_ptr == nullptr) {
        spdlog::error("{}: AddFontFromMemoryTTF failed", path);
        std::terminate();
    }

    return font_ptr;
}

static void on_glfw_framebuffer_size(const io::GlfwFramebufferSizeEvent& event)
{
    if(globals::world_fbo) {
        glDeleteRenderbuffers(1, &globals::world_fbo_depth);
        glDeleteTextures(1, &globals::world_fbo_color);
        glDeleteFramebuffers(1, &globals::world_fbo);
    }

    glGenFramebuffers(1, &globals::world_fbo);
    glGenTextures(1, &globals::world_fbo_color);
    glGenRenderbuffers(1, &globals::world_fbo_depth);

    glBindTexture(GL_TEXTURE_2D, globals::world_fbo_color);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, event.size.x, event.size.y, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

    glBindRenderbuffer(GL_RENDERBUFFER, globals::world_fbo_depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, event.size.x, event.size.y);

    glBindFramebuffer(GL_FRAMEBUFFER, globals::world_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, globals::world_fbo_color, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, globals::world_fbo_depth);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        spdlog::critical("opengl: world framebuffer is incomplete");
        glDeleteRenderbuffers(1, &globals::world_fbo_depth);
        glDeleteTextures(1, &globals::world_fbo_color);
        glDeleteFramebuffers(1, &globals::world_fbo);
        std::terminate();
    }
}

static void on_glfw_key(const io::GlfwKeyEvent& event)
{
    if(!globals::gui_keybind_ptr && hide_hud_toggle.equals(event.key) && (event.action == GLFW_PRESS)) {
        client_game::hide_hud = !client_game::hide_hud;
    }
}

void client_game::init(void)
{
    auto& io = ImGui::GetIO();
    io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = nullptr;
    io.Fonts->Clear();

    ImFontConfig font_config;
    font_config.FontDataOwnedByAtlas = false;

    ImFontGlyphRangesBuilder builder;
    builder.AddRanges(io.Fonts->GetGlyphRangesDefault());
    builder.AddRanges(io.Fonts->GetGlyphRangesCyrillic());

    ImVector<ImWchar> ranges;
    builder.BuildRanges(&ranges);

    globals::font_unscii16 = load_font("fonts/unscii-16.ttf", 16.0f, font_config, ranges);
    globals::font_unscii8 = load_font("fonts/unscii-8.ttf", 8.0f, font_config, ranges);

    gui::client_splash::init();
    gui::client_splash::render();

    globals::client_config.add_value("game.streamer_mode", client_game::streamer_mode);
    globals::client_config.add_value("game.vertical_sync", client_game::vertical_sync);
    globals::client_config.add_value("game.world_curvature", client_game::world_curvature);
    globals::client_config.add_value("game.fog_mode", client_game::fog_mode);
    globals::client_config.add_value("game.username", client_game::username);
    globals::client_config.add_value("game.key.toggle_hide_hud", hide_hud_toggle);

    settings::init();

    settings::add_checkbox(0, client_game::streamer_mode, settings_location::VIDEO_GUI, "game.streamer_mode", true);
    settings::add_checkbox(5, client_game::vertical_sync, settings_location::VIDEO, "game.vertical_sync", false);
    settings::add_checkbox(4, client_game::world_curvature, settings_location::VIDEO, "game.world_curvature", true);
    settings::add_stepper(3, client_game::fog_mode, settings_location::VIDEO, "game.fog_mode", false);
    settings::add_input(1, client_game::username, settings_location::GENERAL, "game.username", true, false);
    settings::add_keybind(4, hide_hud_toggle, settings_location::KEYBOARD_MISC, "game.key.toggle_hide_hud");

    globals::client_host = enet_host_create(nullptr, 1, 1, 0, 0);

    if(!globals::client_host) {
        spdlog::critical("game: unable to setup an ENet host");
        std::terminate();
    }

    gui::language::init();

    session::init();

    entity::player_look::init();
    entity::player_move::init();
    world::player_target::init();

    io::gamepad::init();

    entity::camera::init();

    world::voxel_anims::init();

    world::outline::init();
    world::chunk_mesher::init();
    world::chunk_renderer::init();

    globals::world_fbo = 0;
    globals::world_fbo_color = 0;
    globals::world_fbo_depth = 0;

    world::voxel_sounds::init();

    world::skybox::init();

    ImGuiStyle& style = ImGui::GetStyle();

    // Black buttons on a dark background
    // may be harder to read than the text on them
    style.FrameBorderSize = 1.0;
    style.TabBorderSize = 1.0;

    // Rounding on elements looks cool but I am
    // aiming for a more or less blocky and
    // visually simple HiDPI-friendly UI style
    style.TabRounding = 0.0f;
    style.GrabRounding = 0.0f;
    style.ChildRounding = 0.0f;
    style.FrameRounding = 0.0f;
    style.PopupRounding = 0.0f;
    style.WindowRounding = 0.0f;
    style.ScrollbarRounding = 0.0f;

    style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
    style.Colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    style.Colors[ImGuiCol_Border] = ImVec4(0.79f, 0.79f, 0.79f, 0.50f);
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.54f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.36f, 0.36f, 0.36f, 0.40f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.63f, 0.63f, 0.63f, 0.67f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.00f, 0.00f, 0.00f, 0.75f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.81f, 0.81f, 0.81f, 0.75f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.00f, 0.00f, 0.00f, 0.75f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    style.Colors[ImGuiCol_Separator] = ImVec4(0.49f, 0.49f, 0.49f, 0.50f);
    style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.56f, 0.56f, 0.56f, 0.78f);
    style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.34f, 0.34f, 0.34f, 0.20f);
    style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.57f, 0.57f, 0.57f, 0.67f);
    style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.95f);
    style.Colors[ImGuiCol_Tab] = ImVec4(0.00f, 0.00f, 0.00f, 0.75f);
    style.Colors[ImGuiCol_TabHovered] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    style.Colors[ImGuiCol_TabActive] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.13f, 0.13f, 0.13f, 0.97f);
    style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.44f, 0.44f, 0.44f, 1.00f);
    style.Colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.69f, 0.00f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.00f, 1.00f, 0.20f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
    style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
    style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
    style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
    style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.61f, 0.61f, 0.61f, 0.35f);
    style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

    // Making my own Game UI for Source Engine
    // taught me one important thing: dimensions
    // of UI elements must be calculated at semi-runtime
    // so there's simply no point for an INI file.
    io.IniFilename = nullptr;

    toggles::init();

    gui::background::init();

    gui::scoreboard::init();

    gui::client_chat::init();

    gui::bother::init();

    gui::main_menu::init();
    gui::play_menu::init();
    gui::progress_bar::init();
    gui::message_box::init();
    gui::direct_connection::init();

    gui::crosshair::init();
    gui::hotbar::init();
    gui::metrics::init();
    gui::status_lines::init();

    screenshot::init();

    globals::gui_keybind_ptr = nullptr;
    globals::gui_scale = 0U;
    globals::gui_screen = GUI_MAIN_MENU;

    sound::init_config();

    if(globals::sound_ctx) {
        sound::init();
    }

    client_receive::init();

    experiments::init();

    globals::dispatcher.sink<io::GlfwFramebufferSizeEvent>().connect<&on_glfw_framebuffer_size>();
    globals::dispatcher.sink<io::GlfwKeyEvent>().connect<&on_glfw_key>();
}

void client_game::init_late(void)
{
    toggles::init_late();

    if(globals::sound_ctx) {
        sound::init_late();
    }

    gui::language::init_late();

    settings::init_late();

    gui::client_chat::init_late();

    gui::status_lines::init_late();

    game_voxels::populate();
    game_items::populate();

    std::size_t max_texture_count = 0;

    // Figure out the total texture count
    // NOTE: this is very debug, early and a quite
    // conservative limit choice; there must be a better
    // way to make this limit way smaller than it currently is
    for(const std::shared_ptr<world::VoxelInfo>& info : world::voxel_registry::voxels) {
        for(const world::VoxelTexture& vtex : info->textures) {
            max_texture_count += vtex.paths.size();
        }
    }

    // UNDONE: asset packs for non-16x16 stuff
    world::voxel_atlas::create(16, 16, max_texture_count);

    for(std::shared_ptr<world::VoxelInfo>& info : world::voxel_registry::voxels) {
        for(world::VoxelTexture& vtex : info->textures) {
            if(auto strip = world::voxel_atlas::find_or_load(vtex.paths)) {
                vtex.cached_offset = strip->offset;
                vtex.cached_plane = strip->plane;
                continue;
            }

            spdlog::critical("client_gl: {}: failed to load atlas strips", info->name);
            std::terminate();
        }
    }

    world::voxel_atlas::generate_mipmaps();

    for(std::shared_ptr<world::ItemInfo>& info : world::item_registry::items) {
        info->cached_texture = resource::load<TextureGUI>(info->texture.c_str(), TEXTURE_GUI_LOAD_CLAMP_S | TEXTURE_GUI_LOAD_CLAMP_T);
    }

    experiments::init_late();

    gui::client_splash::init_late();

    gui::window_title::update();
}

void client_game::shutdown(void)
{
    world::voxel_sounds::shutdown();

    experiments::shutdown();

    session::shutdown();

    if(globals::sound_ctx) {
        sound::shutdown();
    }

    gui::hotbar::shutdown();
    gui::main_menu::shutdown();
    gui::play_menu::shutdown();

    gui::bother::shutdown();

    gui::client_chat::shutdown();

    gui::background::shutdown();

    gui::crosshair::shutdown();

    delete globals::dimension;
    globals::player = entt::null;
    globals::dimension = nullptr;

    world::item_registry::purge();
    world::voxel_registry::purge();

    world::voxel_atlas::destroy();

    glDeleteRenderbuffers(1, &globals::world_fbo_depth);
    glDeleteTextures(1, &globals::world_fbo_color);
    glDeleteFramebuffers(1, &globals::world_fbo);

    world::outline::shutdown();
    world::chunk_renderer::shutdown();
    world::chunk_mesher::shutdown();

    enet_host_destroy(globals::client_host);
}

void client_game::fixed_update(void)
{
    entity::player_move::fixed_update();

    // Only update world simulation gamesystems
    // if the player can actually observe all the
    // changes these gamesystems cause visually
    if(session::is_ingame()) {
        entity::Collision::fixed_update(globals::dimension);
        entity::Velocity::fixed_update(globals::dimension);
        entity::Transform::fixed_update(globals::dimension);
        entity::Gravity::fixed_update(globals::dimension);
        entity::Stasis::fixed_update(globals::dimension);
    }
}

void client_game::fixed_update_late(void)
{
    if(session::is_ingame()) {
        const auto& head = globals::dimension->entities.get<entity::Head>(globals::player);
        const auto& transform = globals::dimension->entities.get<entity::Transform>(globals::player);
        const auto& velocity = globals::dimension->entities.get<entity::Velocity>(globals::player);

        protocol::EntityHead head_packet;
        head_packet.entity = entt::null; // ignored by server
        head_packet.angles = head.angles;

        protocol::EntityTransform transform_packet;
        transform_packet.entity = entt::null; // ignored by server
        transform_packet.angles = transform.angles;
        transform_packet.chunk = transform.chunk;
        transform_packet.local = transform.local;

        protocol::EntityVelocity velocity_packet;
        velocity_packet.entity = entt::null; // ignored by server
        velocity_packet.value = velocity.value;

        protocol::send(session::peer, protocol::encode(head_packet));
        protocol::send(session::peer, protocol::encode(transform_packet));
        protocol::send(session::peer, protocol::encode(velocity_packet));
    }
}

void client_game::update(void)
{
    if(session::is_ingame()) {
        if(toggles::get(TOGGLE_PM_FLIGHT)) {
            globals::dimension->entities.remove<entity::Gravity>(globals::player);
        }
        else {
            globals::dimension->entities.emplace_or_replace<entity::Gravity>(globals::player);
        }
    }

    if(globals::sound_ctx) {
        sound::update();

        entity::listener::update();

        entity::SoundEmitter::update();
    }

    entity::interpolation::update();

    world::player_target::update();

    entity::camera::update();

    world::voxel_anims::update();

    world::chunk_mesher::update();

    gui::client_chat::update();

    experiments::update();

    constexpr auto half_base_width = 0.5f * static_cast<float>(BASE_WIDTH);
    constexpr auto half_base_height = 0.5f * static_cast<float>(BASE_HEIGHT);

    auto twice_scale_x = static_cast<float>(globals::width) / half_base_width;
    auto twice_scale_y = static_cast<float>(globals::height) / half_base_height;

    auto scale_x = math::max(1.0f, 0.5f * glm::floor(twice_scale_x));
    auto scale_y = math::max(1.0f, 0.5f * glm::floor(twice_scale_y));
    auto scale_min = math::ceil<unsigned int>(math::min(scale_x, scale_y));
    auto scale_int = math::max(1U, (scale_min / 2U) * 2U);

    auto& io = ImGui::GetIO();
    io.FontGlobalScale = scale_int;
    globals::gui_scale = scale_int;
}

void client_game::update_late(void)
{
    ENetEvent enet_event;

    while(0 < enet_host_service(globals::client_host, &enet_event, 0)) {
        switch(enet_event.type) {
            case ENET_EVENT_TYPE_CONNECT:
                session::send_login_request();
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                session::invalidate();
                break;
            case ENET_EVENT_TYPE_RECEIVE:
                protocol::decode(globals::dispatcher, enet_event.packet, enet_event.peer);
                enet_packet_destroy(enet_event.packet);
                break;
        }
    }

    entity::player_look::update_late();
    entity::player_move::update_late();

    gui::play_menu::update_late();

    gui::bother::update_late();

    experiments::update_late();

    io::gamepad::update_late();

    world::chunk_visibility::update_late();

    if(client_game::vertical_sync.get_value()) {
        glfwSwapInterval(1);
    }
    else {
        glfwSwapInterval(0);
    }
}

void client_game::render(void)
{
    glViewport(0, 0, globals::width, globals::height);
    glBindFramebuffer(GL_FRAMEBUFFER, globals::world_fbo);
    glClearColor(world::skybox::fog_color.r, world::skybox::fog_color.g, world::skybox::fog_color.b, 1.000f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if(globals::dimension) {
        world::chunk_renderer::render();
    }

    glEnable(GL_DEPTH_TEST);

    world::player_target::render();

    if(globals::dimension) {
        auto group = globals::dimension->entities.group(
            entt::get<entity::Player, entity::Collision, entity::client::HeadIntr, entity::client::TransformIntr>);

        world::outline::prepare();

        for(const auto [entity, collision, head, transform] : group.each()) {
            if(entity == globals::player) {
                // Don't render ourselves
                continue;
            }

            glm::fvec3 forward;
            math::vectors(transform.angles + head.angles, forward);
            forward *= 2.0f;

            glm::fvec3 hull_size = collision.aabb.max - collision.aabb.min;
            glm::fvec3 hull_fpos = transform.local + collision.aabb.min;
            glm::fvec3 look = transform.local + head.offset;

            world::outline::cube(transform.chunk, hull_fpos, hull_size, 1.0f, glm::fvec4(1.0f, 0.0f, 0.0f, 1.0f));
            world::outline::line(transform.chunk, look, forward, 1.0f, glm::fvec4(0.9f, 0.9f, 0.9f, 1.0f));
        }
    }

    glEnable(GL_DEPTH_TEST);

    glViewport(0, 0, globals::width, globals::height);
    glClearColor(0.000f, 0.000f, 0.000f, 1.000f);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, globals::world_fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, globals::width, globals::height, 0, 0, globals::width, globals::height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void client_game::layout(void)
{
    if(!session::is_ingame()) {
        gui::background::layout();
    }

    if(!globals::gui_screen || (globals::gui_screen == GUI_CHAT)) {
        if(toggles::get(TOGGLE_METRICS_UI) && !client_game::hide_hud) {
            // This contains Minecraft-esque debug information
            // about the hardware, world state and other
            // things that might be uesful
            gui::metrics::layout();
        }
    }

    if(session::is_ingame()) {
        gui::client_chat::layout();
        gui::scoreboard::layout();

        if(!globals::gui_screen && !client_game::hide_hud) {
            gui::hotbar::layout();
            gui::status_lines::layout();
            gui::crosshair::layout();
        }
    }

    if(globals::gui_screen) {
        if(session::is_ingame() && (globals::gui_screen != GUI_CHAT)) {
            const float width_f = static_cast<float>(globals::width);
            const float height_f = static_cast<float>(globals::height);
            const ImU32 darken = ImGui::GetColorU32(ImVec4(0.00f, 0.00f, 0.00f, 0.75f));
            ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(), ImVec2(width_f, height_f), darken);
        }

        switch(globals::gui_screen) {
            case GUI_MAIN_MENU:
                gui::main_menu::layout();
                break;
            case GUI_PLAY_MENU:
                gui::play_menu::layout();
                break;
            case GUI_SETTINGS:
                settings::layout();
                break;
            case GUI_PROGRESS_BAR:
                gui::progress_bar::layout();
                break;
            case GUI_MESSAGE_BOX:
                gui::message_box::layout();
                break;
            case GUI_DIRECT_CONNECTION:
                gui::direct_connection::layout();
                break;
        }
    }
}
