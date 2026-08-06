#include "server/pch.hh"

#include "core/config/map.hh"
#include "core/core.hh"
#include "core/exception.hh"
#include "core/res/resource.hh"
#include "core/threading.hh"
#include "core/utils/epoch.hh"
#include "core/version.hh"

#include "shared/coord.hh"
#include "shared/game.hh"
#include "shared/splash.hh"

#include "server/constant.hh"
#include "server/game.hh"
#include "server/globals.hh"
#include "server/net/host.hh"
#include "server/net/invites.hh"
#include "server/net/sessions.hh"
#include "server/net/whitelist.hh"
#include "server/system/collector.hh"
#include "server/world/climate.hh"
#include "server/world/worldgen.hh"

static std::atomic_bool s_is_running;

static void signal_handler(int)
{
    LOG_INFO("received termination signal");

    s_is_running.store(false);
}

static void zoned_fixed_update(void)
{
    ZoneScopedN("server::fixed_update");

    shared_game::fixed_update();
    server_game::fixed_update();
}

static void zoned_fixed_update_late(void)
{
    ZoneScopedN("server::fixed_update_late");

    host::fixed_update_late();

    shared_game::fixed_update_late();
    server_game::fixed_update_late();

    collector::fixed_update_late();
}

static void wrapped_main(int argc, char** argv)
{
    core::setup(argc, argv);

    tracy::SetThreadName("Main");
    TracySetProgramName("Voxelius Server");
    TracyAppInfo(version::full.data(), version::full.size());
    TracyPlotConfig("Frametime ms", tracy::PlotFormatType::Number, false, true, 0);

    LOG_INFO("engine version: {}", version::full);

    std::signal(SIGINT, &signal_handler);
    std::signal(SIGTERM, &signal_handler);

    host::init();

    shared_game::init();
    server_game::init();

    collector::init();

    whitelist::init();
    invites::init();
    sessions::init();

    worldgen::init();

    splash::init(SPLASH_SERVER);

    globals::server_config.load("server.conf");

    host::init_late();

    shared_game::init_late();
    server_game::init_late();

    sessions::init_late();

    climate::rebuild();

#ifndef NDEBUG
    LOG_WARNING("debug build");
#endif

    s_is_running.store(true);

    globals::curtime_us = utils::epoch_microseconds();

    globals::fixed_framecount = 0;
    globals::fixed_frametime_us = constant::FIXED_FRAMETIME_US;
    globals::fixed_frametime = static_cast<float>(globals::fixed_frametime_us) / 1000000.0f;
    globals::fixed_frametime_avg = 0.0f;

    auto last_curtime_us = globals::curtime_us;
    auto actual_frametime_us = globals::fixed_frametime_us;
    auto actual_frametime = 0.0f;
    auto next_tick_us = globals::curtime_us;

    while(s_is_running.load()) {
        globals::curtime_us = utils::epoch_microseconds();

        next_tick_us += constant::FIXED_FRAMETIME_US;

        if(globals::curtime_us < next_tick_us) {
            std::this_thread::sleep_for(std::chrono::microseconds(next_tick_us - globals::curtime_us));
        }
        else {
            next_tick_us = globals::curtime_us;
        }

        actual_frametime_us = globals::curtime_us - last_curtime_us;
        actual_frametime = static_cast<float>(actual_frametime_us) / 1000000.0f;
        globals::fixed_frametime_avg += actual_frametime;
        globals::fixed_frametime_avg *= 0.5f;

        last_curtime_us = globals::curtime_us;

        zoned_fixed_update();

        zoned_fixed_update_late();

        FrameMark;

        globals::fixed_framecount += 1;

        globals::dispatcher.update();

        res::soft_purge();

        TracyPlot("Fixed frametime ms", 1000.0f * globals::fixed_frametime);
        TracyPlot("Fixed framerate", static_cast<int64_t>(1.0e6f / globals::fixed_frametime));
    }

    LOG_INFO("shutdown after {} frames", globals::fixed_framecount);
    LOG_INFO("avg tickrate: {:.03f} TPS ({:.03f} MSPT)", 1.0f / globals::fixed_frametime_avg, 1000.0f * globals::fixed_frametime_avg);

    worldgen::shutdown();

    sessions::shutdown();
    invites::shutdown();
    whitelist::shutdown();

    server_game::shutdown();
    shared_game::shutdown();

    host::shutdown();

    res::hard_purge();

    globals::server_config.save("server.conf");

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
        return EXIT_FAILURE;
    }
    catch(const vx::detail::Exception& ex) {
        auto& location = ex.location();
        auto file = std::filesystem::path(location.file_name()).filename().string();
        auto line = static_cast<unsigned long>(location.line());

        uulog::detail::error(file.c_str(), line, ex.what_standard(), std::strlen(ex.what_standard()));
        return EXIT_FAILURE;
    }
    catch(...) {
        uulog::detail::error("unknown", 0, "unknown exception", std::strlen("unknown exception"));
        return EXIT_FAILURE;
    }
}
