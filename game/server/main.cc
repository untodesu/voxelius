#include "server/pch.hh"

#include "core/config/number.hh"
#include "core/io/cmdline.hh"
#include "core/io/config_map.hh"
#include "core/math/constexpr.hh"
#include "core/resource/binfile.hh"
#include "core/resource/image.hh"
#include "core/resource/resource.hh"
#include "core/utils/epoch.hh"

#include "core/version.hh"

#include "shared/game.hh"
#include "shared/protocol.hh"
#include "shared/threading.hh"

#include "server/game.hh"
#include "server/globals.hh"

static config::Unsigned server_tickrate(protocol::TICKRATE, 10U, 300U);

static void on_termination_signal(int)
{
    spdlog::warn("server: received termination signal");
    globals::is_running = false;
}

int main(int argc, char** argv)
{
    io::cmdline::create(argc, argv);

    shared_game::init(argc, argv);

    spdlog::info("Voxelius Server {}", project_version_string);

    globals::fixed_frametime = 0.0f;
    globals::fixed_frametime_avg = 0.0f;
    globals::fixed_frametime_us = 0;
    globals::fixed_framecount = 0;

    globals::curtime = utils::unix_microseconds();

    globals::is_running = true;

    std::signal(SIGINT, &on_termination_signal);
    std::signal(SIGTERM, &on_termination_signal);

    server_game::init();

    threading::init();

    globals::server_config.add_value("server.tickrate", server_tickrate);
    globals::server_config.load_file("server.conf");
    globals::server_config.load_cmdline();

    globals::tickrate = server_tickrate.get_value();
    globals::tickrate_dt = static_cast<std::uint64_t>(1000000.0f / static_cast<float>(globals::tickrate));

    server_game::init_late();

    std::uint64_t last_curtime = globals::curtime;

    while(globals::is_running) {
        globals::curtime = utils::unix_microseconds();

        globals::fixed_frametime_us = globals::curtime - last_curtime;
        globals::fixed_frametime = static_cast<float>(globals::fixed_frametime_us) / 1000000.0f;
        globals::fixed_frametime_avg += globals::fixed_frametime;
        globals::fixed_frametime_avg *= 0.5f;

        last_curtime = globals::curtime;

        server_game::fixed_update();
        server_game::fixed_update_late();

        globals::dispatcher.update();

        globals::fixed_framecount += 1;

        std::this_thread::sleep_for(std::chrono::microseconds(globals::tickrate_dt));

        resource::soft_cleanup<BinFile>();
        resource::soft_cleanup<Image>();

        threading::update();
    }

    server_game::shutdown();

    resource::hard_cleanup<BinFile>();
    resource::hard_cleanup<Image>();

    threading::shutdown();

    spdlog::info("server: shutdown after {} frames", globals::fixed_framecount);
    spdlog::info("server: average framerate: {:.03f} TPS", 1.0f / globals::fixed_frametime_avg);
    spdlog::info("server: average frametime: {:.03f} MSPT", 1000.0f * globals::fixed_frametime_avg);

    globals::server_config.save_file("server.conf");

    shared_game::shutdown();

    return EXIT_SUCCESS;
}
