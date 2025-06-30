#include "shared/pch.hh"

#include "shared/game.hh"

#include "core/io/cmdline.hh"

static std::filesystem::path get_gamepath(void)
{
    if(auto gamepath = io::cmdline::get("gamepath")) {
        // Allow users and third-party launchers to override
        // content location. Perhaps this would work to allow
        // for a Minecraft-like versioning approach?
        return std::filesystem::path(gamepath);
    }

    return std::filesystem::current_path() / "data";
}

static std::filesystem::path get_userpath(void)
{
    if(auto userpath = io::cmdline::get("userpath")) {
        // Allow users and third-party launchers to override
        // user data location. Perhaps this would work to allow
        // for a Minecraft-like versioning approach?
        return std::filesystem::path(userpath);
    }

    if(auto win_appdata = std::getenv("APPDATA")) {
        // On pre-seven Windows systems it's just AppData
        // On post-seven Windows systems it's AppData/Roaming
        return std::filesystem::path(win_appdata) / "voxelius";
    }

    if(auto xdg_home = std::getenv("XDG_DATA_HOME")) {
        // Systems with an active X11/Wayland session
        // theoretically should have this defined, and
        // it can be different from ${HOME} (I think).
        return std::filesystem::path(xdg_home) / ".voxelius";
    }

    if(auto unix_home = std::getenv("HOME")) {
        // Any spherical UNIX/UNIX-like system in vacuum
        // has this defined for every single user process.
        return std::filesystem::path(unix_home) / ".voxelius";
    }

    // Give up and save stuff into CWD
    return std::filesystem::current_path();
}

void shared_game::init(int argc, char** argv)
{
    auto logger = spdlog::default_logger();
    auto& logger_sinks = logger->sinks();

    logger_sinks.clear();
    logger_sinks.push_back(std::make_shared<spdlog::sinks::stderr_color_sink_mt>());
    logger_sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("voxelius.log", false));

#if defined(NDEBUG)
    constexpr auto default_loglevel = spdlog::level::info;
#else
    constexpr auto default_loglevel = spdlog::level::trace;
#endif

    if(io::cmdline::contains("quiet")) {
        logger->set_level(spdlog::level::warn);
    } else if(io::cmdline::contains("verbose")) {
        logger->set_level(spdlog::level::trace);
    } else {
        logger->set_level(default_loglevel);
    }

    logger->set_pattern("%H:%M:%S.%e %^[%L]%$ %v");
    logger->flush();

    if(!PHYSFS_init(argv[0])) {
        spdlog::critical("physfs: init failed: {}", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
        std::terminate();
    }

    auto gamepath = get_gamepath();
    auto userpath = get_userpath();

    spdlog::info("shared_game: set gamepath to {}", gamepath.string());
    spdlog::info("shared_game: set userpath to {}", userpath.string());

    std::error_code ignore_error = {};
    std::filesystem::create_directories(gamepath, ignore_error);
    std::filesystem::create_directories(userpath, ignore_error);

    if(!PHYSFS_mount(gamepath.string().c_str(), nullptr, false)) {
        spdlog::critical("physfs: mount {} failed: {}", gamepath.string(), PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
        std::terminate();
    }

    if(!PHYSFS_mount(userpath.string().c_str(), nullptr, false)) {
        spdlog::critical("physfs: mount {} failed: {}", userpath.string(), PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
        std::terminate();
    }

    if(!PHYSFS_setWriteDir(userpath.string().c_str())) {
        spdlog::critical("physfs: setwritedir {} failed: {}", userpath.string(), PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
        std::terminate();
    }

    if(enet_initialize()) {
        spdlog::critical("enet: init failed");
        std::terminate();
    }
}

void shared_game::shutdown(void)
{
    enet_deinitialize();

    if(!PHYSFS_deinit()) {
        spdlog::critical("physfs: deinit failed: {}", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
        std::terminate();
    }
}
