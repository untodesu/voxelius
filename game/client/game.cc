#include "client/pch.hh"
#include "client/game.hh"

#include "core/angles.hh"
#include "core/binfile.hh"
#include "core/config.hh"
#include "core/feature.hh"
#include "core/resource.hh"

#include "shared/collision.hh"
#include "shared/coord.hh"
#include "shared/dimension.hh"
#include "shared/game_items.hh"
#include "shared/game_voxels.hh"
#include "shared/gravity.hh"
#include "shared/head.hh"
#include "shared/head.hh"
#include "shared/item_registry.hh"
#include "shared/player.hh"
#include "shared/protocol.hh"
#include "shared/ray_dda.hh"
#include "shared/stasis.hh"
#include "shared/transform.hh"
#include "shared/transform.hh"
#include "shared/velocity.hh"
#include "shared/voxel_registry.hh"

#include "client/background.hh"
#include "client/bother.hh"
#include "client/camera.hh"
#include "client/chat.hh"
#include "client/chunk_mesher.hh"
#include "client/chunk_renderer.hh"
#include "client/chunk_visibility.hh"
#include "client/const.hh"
#include "client/crosshair.hh"
#include "client/direct_connection.hh"
#include "client/experiments.hh"
#include "client/gamepad.hh"
#include "client/glfw.hh"
#include "client/globals.hh"
#include "client/gui_screen.hh"
#include "client/hotbar.hh"
#include "client/interpolation.hh"
#include "client/keybind.hh"
#include "client/language.hh"
#include "client/listener.hh"
#include "client/main_menu.hh"
#include "client/message_box.hh"
#include "client/metrics.hh"
#include "client/outline.hh"
#include "client/play_menu.hh"
#include "client/player_look.hh"
#include "client/player_move.hh"
#include "client/player_target.hh"
#include "client/progress_bar.hh"
#include "client/receive.hh"
#include "client/scoreboard.hh"
#include "client/screenshot.hh"
#include "client/session.hh"
#include "client/settings.hh"
#include "client/skybox.hh"
#include "client/sound_emitter.hh"
#include "client/sound.hh"
#include "client/splash.hh"
#include "client/status_lines.hh"
#include "client/texture_gui.hh"
#include "client/toggles.hh"
#include "client/voxel_anims.hh"
#include "client/voxel_atlas.hh"
#include "client/voxel_sounds.hh"
#include "client/window_title.hh"

ConfigBoolean client_game::streamer_mode(false);
ConfigBoolean client_game::vertical_sync(true);
ConfigBoolean client_game::world_curvature(true);
ConfigUnsigned client_game::pixel_size(2U, 1U, 4U);
ConfigUnsigned client_game::fog_mode(1U, 0U, 2U);
ConfigString client_game::username("player");

bool client_game::hide_hud = false;

static ConfigKeyBind hide_hud_toggle(GLFW_KEY_F1);

static resource_ptr<BinFile> bin_unscii16;
static resource_ptr<BinFile> bin_unscii8;

static void on_glfw_framebuffer_size(const GlfwFramebufferSizeEvent &event)
{
    auto width_float = static_cast<float>(event.size.x);
    auto height_float = static_cast<float>(event.size.y);
    auto wscale = cxpr::max(1U, cxpr::floor<unsigned int>(width_float / static_cast<float>(BASE_WIDTH)));
    auto hscale = cxpr::max(1U, cxpr::floor<unsigned int>(height_float / static_cast<float>(BASE_HEIGHT)));
    auto scale = cxpr::min(wscale, hscale);

    if(globals::gui_scale != scale) {
        auto &io = ImGui::GetIO();
        auto &style = ImGui::GetStyle();

        ImFontConfig font_config;
        font_config.FontDataOwnedByAtlas = false;

        io.Fonts->Clear();

        ImFontGlyphRangesBuilder builder;

        // This should cover a hefty range of glyph ranges.
        // UNDONE: just slap the whole UNICODE Plane-0 here?
        builder.AddRanges(io.Fonts->GetGlyphRangesDefault());
        builder.AddRanges(io.Fonts->GetGlyphRangesCyrillic());
        builder.AddRanges(io.Fonts->GetGlyphRangesGreek());
        builder.AddRanges(io.Fonts->GetGlyphRangesJapanese());

        ImVector<ImWchar> ranges = {};
        builder.BuildRanges(&ranges);

        globals::font_default = io.Fonts->AddFontFromMemoryTTF(bin_unscii16->buffer, bin_unscii16->size, 16.0f * scale, &font_config, ranges.Data);
        globals::font_chat = io.Fonts->AddFontFromMemoryTTF(bin_unscii16->buffer, bin_unscii16->size, 8.0f * scale, &font_config, ranges.Data);
        globals::font_debug = io.Fonts->AddFontFromMemoryTTF(bin_unscii8->buffer, bin_unscii8->size, 4.0f * scale, &font_config);

        // Re-assign the default font
        io.FontDefault = globals::font_default;

        // This should be here!!! Just calling Build()
        // on the font atlas does not invalidate internal
        // device objects defined by the implementation!!!
        ImGui_ImplOpenGL3_CreateDeviceObjects();

        if(globals::gui_scale) {
            // Well, ImGuiStyle::ScaleAllSizes indeed takes
            // the scale values as a RELATIVE scaling, not as
            // absolute. So I have to make a special crutch
            style.ScaleAllSizes(static_cast<float>(scale) / static_cast<float>(globals::gui_scale));
        }

        globals::gui_scale = scale;
    }


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

static void on_glfw_key(const GlfwKeyEvent &event)
{
    if(!globals::gui_keybind_ptr && hide_hud_toggle.equals(event.key) && (event.action == GLFW_PRESS)) {
        client_game::hide_hud = !client_game::hide_hud;
    }
}

void client_game::init(void)
{
    bin_unscii16 = resource::load<BinFile>("fonts/unscii-16.ttf");
    bin_unscii8 = resource::load<BinFile>("fonts/unscii-8.ttf");

    if((bin_unscii16 == nullptr) || (bin_unscii8 == nullptr)) {
        spdlog::critical("client_game: font loading failed");
        std::terminate();
    }

    client_splash::init();
    client_splash::render();

    globals::client_config.add_value("game.streamer_mode", client_game::streamer_mode);
    globals::client_config.add_value("game.vertical_sync", client_game::vertical_sync);
    globals::client_config.add_value("game.world_curvature", client_game::world_curvature);
    globals::client_config.add_value("game.pixel_size", client_game::pixel_size);
    globals::client_config.add_value("game.fog_mode", client_game::fog_mode);
    globals::client_config.add_value("game.username", client_game::username);
    globals::client_config.add_value("game.key.toggle_hide_hud", hide_hud_toggle);

    settings::init();

    settings::add_checkbox(0, client_game::streamer_mode, settings_location::VIDEO_GUI, "game.streamer_mode", true);
    settings::add_checkbox(5, client_game::vertical_sync, settings_location::VIDEO, "game.vertical_sync", false);
    settings::add_checkbox(4, client_game::world_curvature, settings_location::VIDEO, "game.world_curvature", true);
    settings::add_slider(1, client_game::pixel_size, settings_location::VIDEO, "game.pixel_size", true);
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

    ImGuiStyle &style = ImGui::GetStyle();

    // Black buttons on a dark background
    // may be harder to read than the text on them
    style.FrameBorderSize = 1.0;
    style.TabBorderSize = 1.0;

    // Rounding on elements looks cool but I am
    // aiming for a more or less blocky and
    // visually simple HiDPI-friendly UI style
    style.TabRounding       = 0.0f;
    style.GrabRounding      = 0.0f;
    style.ChildRounding     = 0.0f;
    style.FrameRounding     = 0.0f;
    style.PopupRounding     = 0.0f;
    style.WindowRounding    = 0.0f;
    style.ScrollbarRounding = 0.0f;

    style.Colors[ImGuiCol_Text]                     = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    style.Colors[ImGuiCol_TextDisabled]             = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    style.Colors[ImGuiCol_WindowBg]                 = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
    style.Colors[ImGuiCol_ChildBg]                  = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_PopupBg]                  = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    style.Colors[ImGuiCol_Border]                   = ImVec4(0.79f, 0.79f, 0.79f, 0.50f);
    style.Colors[ImGuiCol_BorderShadow]             = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_FrameBg]                  = ImVec4(0.00f, 0.00f, 0.00f, 0.54f);
    style.Colors[ImGuiCol_FrameBgHovered]           = ImVec4(0.36f, 0.36f, 0.36f, 0.40f);
    style.Colors[ImGuiCol_FrameBgActive]            = ImVec4(0.63f, 0.63f, 0.63f, 0.67f);
    style.Colors[ImGuiCol_TitleBg]                  = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
    style.Colors[ImGuiCol_TitleBgActive]            = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_TitleBgCollapsed]         = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
    style.Colors[ImGuiCol_MenuBarBg]                = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarBg]              = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    style.Colors[ImGuiCol_ScrollbarGrab]            = ImVec4(0.00f, 0.00f, 0.00f, 0.75f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered]     = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabActive]      = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    style.Colors[ImGuiCol_CheckMark]                = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    style.Colors[ImGuiCol_SliderGrab]               = ImVec4(0.81f, 0.81f, 0.81f, 0.75f);
    style.Colors[ImGuiCol_SliderGrabActive]         = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_Button]                   = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_ButtonHovered]            = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    style.Colors[ImGuiCol_ButtonActive]             = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    style.Colors[ImGuiCol_Header]                   = ImVec4(0.00f, 0.00f, 0.00f, 0.75f);
    style.Colors[ImGuiCol_HeaderHovered]            = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    style.Colors[ImGuiCol_HeaderActive]             = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    style.Colors[ImGuiCol_Separator]                = ImVec4(0.49f, 0.49f, 0.49f, 0.50f);
    style.Colors[ImGuiCol_SeparatorHovered]         = ImVec4(0.56f, 0.56f, 0.56f, 0.78f);
    style.Colors[ImGuiCol_SeparatorActive]          = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    style.Colors[ImGuiCol_ResizeGrip]               = ImVec4(0.34f, 0.34f, 0.34f, 0.20f);
    style.Colors[ImGuiCol_ResizeGripHovered]        = ImVec4(0.57f, 0.57f, 0.57f, 0.67f);
    style.Colors[ImGuiCol_ResizeGripActive]         = ImVec4(1.00f, 1.00f, 1.00f, 0.95f);
    style.Colors[ImGuiCol_Tab]                      = ImVec4(0.00f, 0.00f, 0.00f, 0.75f);
    style.Colors[ImGuiCol_TabHovered]               = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    style.Colors[ImGuiCol_TabActive]                = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    style.Colors[ImGuiCol_TabUnfocused]             = ImVec4(0.13f, 0.13f, 0.13f, 0.97f);
    style.Colors[ImGuiCol_TabUnfocusedActive]       = ImVec4(0.44f, 0.44f, 0.44f, 1.00f);
    style.Colors[ImGuiCol_PlotLines]                = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    style.Colors[ImGuiCol_PlotLinesHovered]         = ImVec4(0.69f, 0.00f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogram]            = ImVec4(0.00f, 1.00f, 0.20f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogramHovered]     = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_TableHeaderBg]            = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
    style.Colors[ImGuiCol_TableBorderStrong]        = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
    style.Colors[ImGuiCol_TableBorderLight]         = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
    style.Colors[ImGuiCol_TableRowBg]               = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_TableRowBgAlt]            = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
    style.Colors[ImGuiCol_TextSelectedBg]           = ImVec4(0.61f, 0.61f, 0.61f, 0.35f);
    style.Colors[ImGuiCol_DragDropTarget]           = ImVec4(1.00f, 1.00f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_NavHighlight]             = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    style.Colors[ImGuiCol_NavWindowingHighlight]    = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    style.Colors[ImGuiCol_NavWindowingDimBg]        = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    style.Colors[ImGuiCol_ModalWindowDimBg]         = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

    // Making my own Game UI for Source Engine
    // taught me one important thing: dimensions
    // of UI elements must be calculated at semi-runtime
    // so there's simply no point for an INI file.
    ImGui::GetIO().IniFilename = nullptr;

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

    sound::init();

    client_receive::init();

    experiments::init();

    globals::dispatcher.sink<GlfwFramebufferSizeEvent>().connect<&on_glfw_framebuffer_size>();
    globals::dispatcher.sink<GlfwKeyEvent>().connect<&on_glfw_key>();
}

void client_game::init_late(void)
{
    sound::init_late();

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
    for(const std::shared_ptr<VoxelInfo> &info : voxel_registry::voxels) {
        for(const VoxelTexture &vtex : info->textures) {
            max_texture_count += vtex.paths.size();
        }
    }

    // UNDONE: asset packs for non-16x16 stuff
    voxel_atlas::create(16, 16, max_texture_count);

    for(std::shared_ptr<VoxelInfo> &info : voxel_registry::voxels) {
        for(VoxelTexture &vtex : info->textures) {
            if(auto strip = voxel_atlas::find_or_load(vtex.paths)) {
                vtex.cached_offset = strip->offset;
                vtex.cached_plane = strip->plane;
                continue;
            }
            
            spdlog::critical("client_gl: {}: failed to load atlas strips", info->name);
            std::terminate();
        }
    }

    voxel_atlas::generate_mipmaps();

    for(std::shared_ptr<ItemInfo> &info : item_registry::items) {
        info->cached_texture = resource::load<TextureGUI>(info->texture.c_str(), TEXTURE_GUI_LOAD_CLAMP_S | TEXTURE_GUI_LOAD_CLAMP_T);
    }

    experiments::init_late();

    client_splash::init_late();

    window_title::update();
}

void client_game::deinit(void)
{
    voxel_sounds::deinit();

    experiments::deinit();

    session::deinit();

    sound::deinit();

    hotbar::deinit();
    main_menu::deinit();
    play_menu::deinit();

    bother::deinit();

    client_chat::deinit();

    background::deinit();

    crosshair::deinit();

    delete globals::dimension;
    globals::player = entt::null;
    globals::dimension = nullptr;

    item_registry::purge();
    voxel_registry::purge();

    voxel_atlas::destroy();

    glDeleteRenderbuffers(1, &globals::world_fbo_depth);
    glDeleteTextures(1, &globals::world_fbo_color);
    glDeleteFramebuffers(1, &globals::world_fbo);

    outline::deinit();
    chunk_renderer::deinit();
    chunk_mesher::deinit();

    enet_host_destroy(globals::client_host);

    bin_unscii8 = nullptr;
    bin_unscii16 = nullptr;
}

void client_game::fixed_update(void)
{
    player_move::fixed_update();

    // Only update world simulation gamesystems
    // if the player can actually observe all the
    // changes these gamesystems cause visually
    if(session::is_ingame()) {
        CollisionComponent::fixed_update(globals::dimension);
        VelocityComponent::fixed_update(globals::dimension);
        TransformComponent::fixed_update(globals::dimension);
        GravityComponent::fixed_update(globals::dimension);
        StasisComponent::fixed_update(globals::dimension);
    }
}

void client_game::fixed_update_late(void)
{
    if(session::is_ingame()) {
        const auto &head = globals::dimension->entities.get<HeadComponent>(globals::player);
        const auto &transform = globals::dimension->entities.get<TransformComponent>(globals::player);
        const auto &velocity = globals::dimension->entities.get<VelocityComponent>(globals::player);

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
    sound::update();
    listener::update();

    interpolation::update();

    player_target::update();

    camera::update();

    SoundEmitterComponent::update();

    voxel_anims::update();

    chunk_mesher::update();

    chunk_visibility::update();
    
    client_chat::update();

    experiments::update();
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

    if(client_game::vertical_sync.get_value())
        glfwSwapInterval(1);
    else glfwSwapInterval(0);
}

void client_game::render(void)
{
    auto scaled_width = globals::width / cxpr::max<int>(1, client_game::pixel_size.get_value());
    auto scaled_height = globals::height / cxpr::max<int>(1, client_game::pixel_size.get_value());

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
        auto group = globals::dimension->entities.group(entt::get<PlayerComponent, CollisionComponent, HeadComponentIntr, TransformComponentIntr>);

        outline::prepare();

        for(const auto [entity, collision, head, transform] : group.each()) {
            if(entity == globals::player) {
                // Don't render ourselves
                continue;
            }
    
            glm::fvec3 forward;
            cxangles::vectors(transform.angles + head.angles, forward);
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
        if(toggles::draw_metrics && !client_game::hide_hud) {
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
            const float width_f = static_cast<float>(globals::width);
            const float height_f = static_cast<float>(globals::height);
            const ImU32 darken = ImGui::GetColorU32(ImVec4(0.00f, 0.00f, 0.00f, 0.75f));
            ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(), ImVec2(width_f, height_f), darken);
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
