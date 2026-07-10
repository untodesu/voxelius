#include "client/pch.hh"

#include "core/config/map.hh"

#include "core/res/image.hh"
#include "core/res/resource.hh"

#include "core/utils/epoch.hh"

#include "core/core.hh"
#include "core/exception.hh"
#include "core/threading.hh"
#include "core/version.hh"

#include "shared/res/block_collision.hh"
#include "shared/res/block_model.hh"

#include "shared/block_registry.hh"
#include "shared/mod_loader.hh"

#include "client/res/texture2D.hh"

#include "client/block_atlas.hh"
#include "client/block_models.hh"
#include "client/chunk_mesher.hh"

#include "client/frame.hh"
#include "client/game.hh"
#include "client/globals.hh"
#include "client/video.hh"

static std::atomic_bool s_is_running;

static void signal_handler(int)
{
    LOG_INFO("received termination signal");

    s_is_running.store(false);
}

static void handle_events(void)
{
    thread_local SDL_Event event;

    while(SDL_PollEvent(&event)) {
        if(event.type == SDL_EVENT_QUIT) {
            s_is_running.store(false);
            return;
        }

        switch(event.type) {
            case SDL_EVENT_KEY_DOWN:
            case SDL_EVENT_KEY_UP:
                globals::dispatcher.trigger(static_cast<const SDL_KeyboardEvent&>(event.key));
                break;

            case SDL_EVENT_MOUSE_MOTION:
                globals::dispatcher.trigger(static_cast<const SDL_MouseMotionEvent&>(event.motion));
                break;

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL_EVENT_MOUSE_BUTTON_UP:
                globals::dispatcher.trigger(static_cast<const SDL_MouseButtonEvent&>(event.button));
                break;

            case SDL_EVENT_MOUSE_WHEEL:
                globals::dispatcher.trigger(static_cast<const SDL_MouseWheelEvent&>(event.wheel));
                break;

            case SDL_EVENT_WINDOW_MOVED:
            case SDL_EVENT_WINDOW_RESIZED:
            case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
            case SDL_EVENT_WINDOW_METAL_VIEW_RESIZED:
            case SDL_EVENT_WINDOW_MINIMIZED:
            case SDL_EVENT_WINDOW_MAXIMIZED:
            case SDL_EVENT_WINDOW_RESTORED:
            case SDL_EVENT_WINDOW_MOUSE_ENTER:
            case SDL_EVENT_WINDOW_MOUSE_LEAVE:
            case SDL_EVENT_WINDOW_FOCUS_GAINED:
            case SDL_EVENT_WINDOW_FOCUS_LOST:
            case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            case SDL_EVENT_WINDOW_HIT_TEST:
            case SDL_EVENT_WINDOW_ICCPROF_CHANGED:
            case SDL_EVENT_WINDOW_DISPLAY_CHANGED:
            case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED:
            case SDL_EVENT_WINDOW_SAFE_AREA_CHANGED:
            case SDL_EVENT_WINDOW_OCCLUDED:
            case SDL_EVENT_WINDOW_ENTER_FULLSCREEN:
            case SDL_EVENT_WINDOW_LEAVE_FULLSCREEN:
            case SDL_EVENT_WINDOW_DESTROYED:
            case SDL_EVENT_WINDOW_HDR_STATE_CHANGED:
                globals::dispatcher.trigger(static_cast<const SDL_WindowEvent&>(event.window));
                break;
        }

        globals::dispatcher.trigger(static_cast<const SDL_Event&>(event));

        ImGui_ImplSDL3_ProcessEvent(&event);
    }
}

static void wrapped_main(int argc, char** argv)
{
    core::setup(argc, argv);

    LOG_INFO("engine version: {}", version::full);

    std::signal(SIGINT, &signal_handler);
    std::signal(SIGTERM, &signal_handler);

    vx::throw_if_not_fmt(SDL_Init(SDL_INIT_EVENTS), "SDL_Init failed: {}", SDL_GetError());

    Image::register_resource();
    BlockCollision::register_resource();
    BlockModel::register_resource();
    Texture2D::register_resource();

    mod_loader::init();

    // test

    auto definitions = block_registry::all_definitions();

    LOG_INFO("block dump: {} definition(s) (including the reserved BLOCK_ID_NULL slot)", definitions.size());

    for(block_id_type id = 1; id < definitions.size(); ++id) {
        const auto& def = definitions[id];
        auto name = block_registry::name_of(id);

        LOG_INFO("block #{} ({}): render={} health={} emission={} dissipation={} touch={} tags={:#04x} family={} drops={}", id,
            name ? name->full_string() : "unknown", static_cast<unsigned>(def.render), def.health, static_cast<unsigned>(def.emission),
            static_cast<unsigned>(def.dissipation), static_cast<unsigned>(def.touch), static_cast<unsigned>(def.tags), def.family,
            def.drops.size());
    }

    // test

    video::init();

    // TODO: game_ui::init();

    block_atlas::init();

    client_game::init();

    globals::client_config.load("client.conf");
    globals::client_config.load("client.user.conf");

    video::init_late();

    // TODO: game_ui::init_late();

    // Needs both globals::gpu_device (from video::init()) and a
    // committed block_registry (from mod_loader::init()), both of
    // which are guaranteed by this point
    block_atlas::init_late();
    block_models::init_late();

    chunk_mesher::init();

    client_game::init_late();

#ifndef NDEBUG
    LOG_WARNING("debug build");
#endif

    s_is_running.store(true);

    globals::fixed_frametime = 0.0f;
    globals::fixed_frametime_avg = 0.0f;
    globals::fixed_frametime_us = UINT64_MAX;
    globals::fixed_framecount = 0;

    globals::curtime_us = utils::epoch_microseconds();

    globals::window_framecount = 0;
    globals::window_frametime_us = 0;
    globals::window_frametime = 0.0f;
    globals::window_frametime_avg = 0.0f;

    auto last_curtime_us = globals::curtime_us;

    while(s_is_running.load()) {
        globals::curtime_us = utils::epoch_microseconds();

        globals::window_frametime_us = globals::curtime_us - last_curtime_us;
        globals::window_frametime = 1.0e-6f * static_cast<float>(globals::window_frametime_us);
        globals::window_frametime_avg += globals::window_frametime;
        globals::window_frametime_avg *= 0.5f;

        if(globals::fixed_frametime_us == UINT64_MAX) {
            globals::fixed_framecount = 0;
            globals::fixed_accumulator_us = 0;
        }
        else {
            globals::fixed_accumulator_us += globals::window_frametime_us;
            globals::fixed_framecount = globals::fixed_accumulator_us / globals::fixed_frametime_us;
            globals::fixed_accumulator_us %= globals::fixed_frametime_us;
        }

        globals::num_draw_calls = 0;
        globals::num_draw_vertices = 0;

        last_curtime_us = globals::curtime_us;

        handle_events();

        for(std::uint64_t i = 0; i < globals::fixed_framecount; ++i) {
            client_game::fixed_update();
        }

        video::update();

        client_game::update();

        chunk_mesher::update();

        // TODO: game_ui::update();

        if(frame::prepare()) {
            // TODO: client_game::render();
            // TODO: game_ui::layout();

            client_game::layout();

            frame::present();
        }

        for(std::uint64_t i = 0; i < globals::fixed_framecount; ++i) {
            client_game::fixed_update_late();
        }

        video::update_late();

        client_game::update_late();

        threading::update();

        globals::window_framecount += 1;

        globals::dispatcher.update();

        res::soft_purge();
    }

    LOG_INFO("shutdown after {} frames", globals::window_framecount);
    LOG_INFO("avg framerate: {:.03f} FPS ({:.03f} ms)", 1.0f / globals::window_frametime_avg, 1000.0f * globals::window_frametime_avg);

    block_models::shutdown();
    block_atlas::shutdown();

    client_game::shutdown();

    // TODO: game_ui::shutdown();

    res::hard_purge();

    video::shutdown();

    mod_loader::shutdown();

    globals::client_config.save("client.conf");

    core::teardown();
}

int main(int argc, char** argv)
{
    uulog::add_sink(&uulog::builtin::stderr_ansi);

    try {
        wrapped_main(argc, argv);
        return EXIT_SUCCESS;
    }
    catch(const std::exception& ex) {
        uulog::detail::error("unknown", 0, ex.what(), std::strlen(ex.what()));
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Engine Error", ex.what(), nullptr);
        return EXIT_FAILURE;
    }
    catch(const vx::detail::Exception& ex) {
        const auto& location = ex.location();
        const auto file = std::filesystem::path(location.file_name()).filename().string();
        const auto line = static_cast<unsigned long>(location.line());

        uulog::detail::error(file.c_str(), line, ex.what_standard(), ex.what().size());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Engine Error", ex.what_standard(), nullptr);
        return EXIT_FAILURE;
    }
    catch(...) {
        uulog::detail::error("unknown", 0, "Unknown exception", std::strlen("Unknown exception"));
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Engine Error", "Unknown exception", nullptr);
        return EXIT_FAILURE;
    }
}
