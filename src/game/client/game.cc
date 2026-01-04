// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: game.cc
// Description: Main game logic

#include "client/pch.hh"

#include "client/game.hh"

#include "core/config/boolean.hh"
#include "core/config/number.hh"
#include "core/config/string.hh"

#include "core/io/config_map.hh"

#include "core/math/angles.hh"

#include "core/resource/resource.hh"

#include "core/io/physfs.hh"

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

#include "client/io/gamepad.hh"
#include "client/io/keyboard.hh"
#include "client/io/sound.hh"
#include "client/io/video.hh"

#include "client/resource/texture_gui.hh"

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

constexpr static int PIXEL_SIZE = 2;

config::Boolean client_game::streamer_mode(false);
config::Boolean client_game::world_curvature(true);
config::Unsigned client_game::fog_mode(1U, 0U, 2U);
config::String client_game::username("player");

bool client_game::hide_hud = false;

static config::KeyBind hide_hud_toggle(GLFW_KEY_F1);

static int scaled_width;
static int scaled_height;

static ImFont* load_font(std::string_view path, float size, ImFontConfig& font_config, ImVector<ImWchar>& ranges)
{
    std::vector<std::byte> font;

    if(!physfs::read_file(path, font)) {
        spdlog::error("{}: utils::physfs::read_file failed", path);
        std::terminate();
    }

    auto& io = ImGui::GetIO();
    auto font_ptr = io.Fonts->AddFontFromMemoryTTF(font.data(), static_cast<int>(font.size()), size, &font_config, ranges.Data);

    if(font_ptr == nullptr) {
        spdlog::error("{}: AddFontFromMemoryTTF failed", path);
        std::terminate();
    }

    return font_ptr;
}

static void on_framebuffer_size(const FramebufferSizeEvent& event)
{
    if(globals::world_fbo) {
        glDeleteRenderbuffers(1, &globals::world_fbo_depth);
        glDeleteTextures(1, &globals::world_fbo_color);
        glDeleteFramebuffers(1, &globals::world_fbo);
    }

    glGenFramebuffers(1, &globals::world_fbo);
    glGenTextures(1, &globals::world_fbo_color);
    glGenRenderbuffers(1, &globals::world_fbo_depth);

    scaled_width = event.wide() / PIXEL_SIZE;
    scaled_height = event.tall() / PIXEL_SIZE;

    glBindTexture(GL_TEXTURE_2D, globals::world_fbo_color);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, scaled_width, scaled_height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

    glBindRenderbuffer(GL_RENDERBUFFER, globals::world_fbo_depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, scaled_width, scaled_height);

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

static void on_key(const KeyEvent& event)
{
    if(!globals::gui_keybind_ptr && hide_hud_toggle.equals(event.keycode()) && (event.is_action(GLFW_PRESS))) {
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

    client_splash::init();
    client_splash::render();

    globals::client_config.add_value("game.streamer_mode", client_game::streamer_mode);
    globals::client_config.add_value("game.world_curvature", client_game::world_curvature);
    globals::client_config.add_value("game.fog_mode", client_game::fog_mode);
    globals::client_config.add_value("game.username", client_game::username);
    globals::client_config.add_value("game.key.toggle_hide_hud", hide_hud_toggle);

    settings::init();

    settings::add_checkbox(0, client_game::streamer_mode, settings_location::VIDEO_GUI, "game.streamer_mode", true);
    settings::add_checkbox(4, client_game::world_curvature, settings_location::VIDEO, "game.world_curvature", true);
    settings::add_stepper(3, client_game::fog_mode, settings_location::VIDEO, "game.fog_mode", false);
    settings::add_input(1, client_game::username, settings_location::GENERAL, "game.username", true, false);
    settings::add_keybind(4, hide_hud_toggle, settings_location::KEYBOARD_MISC, "game.key.toggle_hide_hud");

    globals::client_host = enet_host_create(nullptr, 1, 1, 0, 0);

    if(!globals::client_host) {
        spdlog::critical("game: unable to setup an ENet host");
        std::terminate();
    }

    language::init();

    session::init();

    player_look::init();
    player_move::init();
    player_target::init();

    gamepad::init();

    camera::init();

    voxel_anims::init();

    outline::init();
    chunk_mesher::init();
    chunk_renderer::init();

    globals::world_fbo = 0;
    globals::world_fbo_color = 0;
    globals::world_fbo_depth = 0;

    voxel_sounds::init();

    skybox::init();

    ImGuiStyle& style = ImGui::GetStyle();

    // Black buttons on a dark background
    // may be harder to read than the text on them
    style.FrameBorderSize = 1.0;
    style.TabBorderSize = 1.0;

    // Rounding on elements looks cool but I am
    // aiming for a more or less blocky and
    // visually simple HiDPI-friendly UI style
    style.ChildRounding = 0.0f;
    style.FrameRounding = 0.0f;
    style.GrabRounding = 0.0f;
    style.PopupRounding = 0.0f;
    style.ScrollbarRounding = 0.0f;
    style.TabRounding = 0.0f;
    style.WindowRounding = 0.0f;

    style.Colors[ImGuiCol_Border] = ImVec4(0.79f, 0.79f, 0.79f, 0.50f);
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    style.Colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.38f);
    style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.54f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.63f, 0.63f, 0.63f, 0.67f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.36f, 0.36f, 0.36f, 0.40f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.00f, 0.00f, 0.00f, 0.75f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    style.Colors[ImGuiCol_NavCursor] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.00f, 1.00f, 0.20f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.69f, 0.00f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.34f, 0.34f, 0.34f, 0.20f);
    style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.95f);
    style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.57f, 0.57f, 0.57f, 0.67f);
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.00f, 0.00f, 0.00f, 0.75f);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    style.Colors[ImGuiCol_Separator] = ImVec4(0.49f, 0.49f, 0.49f, 0.50f);
    style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.56f, 0.56f, 0.56f, 0.78f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.81f, 0.81f, 0.81f, 0.75f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_Tab] = ImVec4(0.00f, 0.00f, 0.00f, 0.75f);
    style.Colors[ImGuiCol_TabActive] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    style.Colors[ImGuiCol_TabHovered] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
    style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
    style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
    style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
    style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.13f, 0.13f, 0.13f, 0.97f);
    style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.44f, 0.44f, 0.44f, 1.00f);
    style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.61f, 0.61f, 0.61f, 0.35f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
    style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.75f);

    io.IniFilename = nullptr;

    toggles::init();

    background::init();

    scoreboard::init();

    client_chat::init();

    bother::init();

    main_menu::init();
    play_menu::init();
    progress_bar::init();
    message_box::init();
    direct_connection::init();

    crosshair::init();
    hotbar::init();
    metrics::init();
    status_lines::init();

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

    globals::dispatcher.sink<FramebufferSizeEvent>().connect<&on_framebuffer_size>();
    globals::dispatcher.sink<KeyEvent>().connect<&on_key>();
}

void client_game::init_late(void)
{
    toggles::init_late();

    if(globals::sound_ctx) {
        sound::init_late();
    }

    language::init_late();

    settings::init_late();

    client_chat::init_late();

    status_lines::init_late();

    game_voxels::populate();
    game_items::populate();

    std::size_t max_texture_count = 0;

    // Figure out the total texture count
    // NOTE: this is very debug, early and a quite
    // conservative limit choice; there must be a better
    // way to make this limit way smaller than it currently is
    for(const auto& voxel : voxel_registry::voxels) {
        max_texture_count += voxel->get_default_textures().size();
        max_texture_count += voxel->get_face_textures(VFACE_NORTH).size();
        max_texture_count += voxel->get_face_textures(VFACE_SOUTH).size();
        max_texture_count += voxel->get_face_textures(VFACE_EAST).size();
        max_texture_count += voxel->get_face_textures(VFACE_WEST).size();
        max_texture_count += voxel->get_face_textures(VFACE_TOP).size();
        max_texture_count += voxel->get_face_textures(VFACE_BOTTOM).size();
        max_texture_count += voxel->get_face_textures(VFACE_CROSS_NWSE).size();
        max_texture_count += voxel->get_face_textures(VFACE_CROSS_NESW).size();
    }

    // UNDONE: asset packs for non-16x16 stuff
    voxel_atlas::create(16, 16, max_texture_count);

    for(auto& voxel : voxel_registry::voxels) {
        constexpr std::array faces = {
            VFACE_NORTH,
            VFACE_SOUTH,
            VFACE_EAST,
            VFACE_WEST,
            VFACE_TOP,
            VFACE_BOTTOM,
            VFACE_CROSS_NWSE,
            VFACE_CROSS_NESW,
        };

        for(auto face : faces) {
            if(auto strip = voxel_atlas::find_or_load(voxel->get_face_textures(face))) {
                voxel->set_face_cache(face, strip->offset, strip->plane);
                continue;
            }

            spdlog::critical("client_gl: {}: failed to load atlas strips", voxel->get_name());
            std::terminate();
        }
    }

    voxel_atlas::generate_mipmaps();

    for(auto& item : item_registry::items) {
        item->set_cached_texture(resource::load<TextureGUI>(item->get_texture(), TEXTURE_GUI_LOAD_CLAMP_S | TEXTURE_GUI_LOAD_CLAMP_T));
    }

    experiments::init_late();

    client_splash::init_late();

    video::update_window_title();
}

void client_game::shutdown(void)
{
    voxel_sounds::shutdown();

    experiments::shutdown();

    session::shutdown();

    if(globals::sound_ctx) {
        sound::shutdown();
    }

    hotbar::shutdown();
    main_menu::shutdown();
    play_menu::shutdown();

    bother::shutdown();

    client_chat::shutdown();

    background::shutdown();

    crosshair::shutdown();

    delete globals::dimension;
    globals::player = entt::null;
    globals::dimension = nullptr;

    item_registry::purge();
    voxel_registry::purge();

    voxel_atlas::destroy();

    glDeleteRenderbuffers(1, &globals::world_fbo_depth);
    glDeleteTextures(1, &globals::world_fbo_color);
    glDeleteFramebuffers(1, &globals::world_fbo);

    outline::shutdown();
    chunk_renderer::shutdown();
    chunk_mesher::shutdown();

    enet_host_destroy(globals::client_host);
}

void client_game::fixed_update(void)
{
    player_move::fixed_update();

    // Only update world simulation gamesystems
    // if the player can actually observe all the
    // changes these gamesystems cause visually
    if(session::is_ingame()) {
        Collision::fixed_update(globals::dimension);
        Velocity::fixed_update(globals::dimension);
        Transform::fixed_update(globals::dimension);
        Gravity::fixed_update(globals::dimension);
        Stasis::fixed_update(globals::dimension);
    }
}

void client_game::fixed_update_late(void)
{
    if(session::is_ingame()) {
        const auto& head = globals::dimension->entities.get<Head>(globals::player);
        const auto& transform = globals::dimension->entities.get<Transform>(globals::player);
        const auto& velocity = globals::dimension->entities.get<Velocity>(globals::player);

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
            globals::dimension->entities.remove<Gravity>(globals::player);
        }
        else {
            globals::dimension->entities.emplace_or_replace<Gravity>(globals::player);
        }
    }

    if(globals::sound_ctx) {
        sound::update();

        listener::update();

        SoundEmitter::update();
    }

    interpolation::update();

    player_target::update();

    camera::update();

    voxel_anims::update();

    chunk_mesher::update();

    client_chat::update();

    experiments::update();

    auto scale_x = glm::max(1.0f, glm::floor(static_cast<float>(globals::width) / static_cast<float>(BASE_WIDTH)));
    auto scale_y = glm::max(1.0f, glm::floor(static_cast<float>(globals::height) / static_cast<float>(BASE_HEIGHT)));
    auto scale_min = glm::min(scale_x, scale_y);

    auto& io = ImGui::GetIO();
    io.FontGlobalScale = scale_min;
    globals::gui_scale = static_cast<unsigned int>(scale_min);
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

    player_look::update_late();
    player_move::update_late();

    play_menu::update_late();

    bother::update_late();

    experiments::update_late();

    gamepad::update_late();

    chunk_visibility::update_late();
}

void client_game::render(void)
{
    glViewport(0, 0, scaled_width, scaled_height);
    glBindFramebuffer(GL_FRAMEBUFFER, globals::world_fbo);
    glClearColor(skybox::fog_color.r, skybox::fog_color.g, skybox::fog_color.b, 1.000f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if(globals::dimension) {
        chunk_renderer::render();
    }

    glEnable(GL_DEPTH_TEST);

    player_target::render();

    if(globals::dimension) {
        auto group = globals::dimension->entities.group(entt::get<Player, Collision, client::HeadIntr, client::TransformIntr>);

        outline::prepare();

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

            outline::cube(transform.chunk, hull_fpos, hull_size, 1.0f, glm::fvec4(1.0f, 0.0f, 0.0f, 1.0f));
            outline::line(transform.chunk, look, forward, 1.0f, glm::fvec4(0.9f, 0.9f, 0.9f, 1.0f));
        }
    }

    glEnable(GL_DEPTH_TEST);

    glViewport(0, 0, globals::width, globals::height);
    glClearColor(0.000f, 0.000f, 0.000f, 1.000f);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, globals::world_fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, scaled_width, scaled_height, 0, 0, globals::width, globals::height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void client_game::layout(void)
{
    if(!session::is_ingame()) {
        background::layout();
    }

    if(!globals::gui_screen || (globals::gui_screen == GUI_CHAT)) {
        if(toggles::get(TOGGLE_METRICS_UI) && !client_game::hide_hud) {
            // This contains Minecraft-esque debug information
            // about the hardware, world state and other
            // things that might be uesful
            metrics::layout();
        }
    }

    if(session::is_ingame()) {
        client_chat::layout();
        scoreboard::layout();

        if(!globals::gui_screen && !client_game::hide_hud) {
            hotbar::layout();
            status_lines::layout();
            crosshair::layout();
        }
    }

    if(globals::gui_screen) {
        if(session::is_ingame() && (globals::gui_screen != GUI_CHAT)) {
            auto width_f = static_cast<float>(globals::width);
            auto height_f = static_cast<float>(globals::height);
            auto darken = ImGui::GetColorU32(ImVec4(0.00f, 0.00f, 0.00f, 0.75f));
            auto darker = ImGui::GetColorU32(ImVec4(0.00f, 0.00f, 0.00f, 0.95f));
            auto draw_list = ImGui::GetBackgroundDrawList();

            draw_list->AddRectFilledMultiColor({}, { width_f, height_f }, darker, darken, darken, darker);
        }

        switch(globals::gui_screen) {
            case GUI_MAIN_MENU:
                main_menu::layout();
                break;
            case GUI_PLAY_MENU:
                play_menu::layout();
                break;
            case GUI_SETTINGS:
                settings::layout();
                break;
            case GUI_PROGRESS_BAR:
                progress_bar::layout();
                break;
            case GUI_MESSAGE_BOX:
                message_box::layout();
                break;
            case GUI_DIRECT_CONNECTION:
                direct_connection::layout();
                break;
        }
    }
}
