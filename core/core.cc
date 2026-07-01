#include "core/pch.hh"

#include "core/core.hh"

#include "core/cmd/builtin.hh"
#include "core/cmd/cmd.hh"

#include "core/config/config.hh"

#include "core/utils/physfs.hh"

#include "core/cmdline.hh"
#include "core/exception.hh"

static std::filesystem::path s_basepath;
static std::filesystem::path s_gamepath;
static std::filesystem::path s_userpath;

const std::filesystem::path& core::basepath(void)
{
    return s_basepath;
}

const std::filesystem::path& core::gamepath(void)
{
    return s_gamepath;
}

const std::filesystem::path& core::userpath(void)
{
    return s_userpath;
}

void core::setup(int argc, char** argv)
{
    cmdline::create(argc, argv);

    auto physfs_init_ok = PHYSFS_init(argv[0]);
    vx::throw_if_not_fmt(physfs_init_ok, "failed to initialize physfs: {}", utils::physfs_error());

    auto enet_init_fail = static_cast<bool>(enet_initialize());
    vx::throw_if<vx::runtime_error>(enet_init_fail, "failed to initialize enet");

    if(cmdline::contains("devel")) {
        s_basepath = std::filesystem::absolute(cmdline::value_or("base", "data/vgame"));
    }
    else {
        s_basepath = std::filesystem::absolute(cmdline::value_or("base", "vgame"));
    }

    s_gamepath = std::filesystem::absolute(cmdline::value_or("game", s_basepath.string()));

    if(auto value = cmdline::value_or_cstr("user", nullptr)) {
        // If there is a third-party launcher that supports
        // multiple profiles, it might throw in a custom user
        // directory with different user-specific configuration files
        s_userpath = std::filesystem::absolute(value);
    }
    else if(auto windows_appdata = std::getenv("APPDATA")) {
        // We appear to run on a system/environment that is itself
        // a Windows or pretends to be Windows; regardless, persistent
        // application data for the game is stored in %APPDATA% in this case
        s_userpath = std::filesystem::path(windows_appdata) / "QFortress" / s_gamepath.filename();
    }
    else if(auto xdg_data_home = std::getenv("XDG_DATA_HOME")) {
        // We appear to run on a system/environment that complies
        // or tries to comply with freedesktop spec; by default XDG_DATA_HOME
        // should resolve to ${HOME}/.local/share so it's a good place for userpath
        s_userpath = std::filesystem::path(xdg_data_home) / "qfortress" / s_gamepath.filename();
    }
    else if(auto unix_home = std::getenv("HOME")) {
        // If anything else fails, as far as I'm concerned, every UNIX system
        // defines an environment variable for user's home directory; we can
        // assume default location from freedesktop spec and put game data there
        s_userpath = std::filesystem::path(unix_home) / ".local/share/qfortress" / s_gamepath.filename();
    }
    else {
        // Give up and save stuff into cwd
        s_userpath = std::filesystem::current_path();
    }

    LOG_DEBUG("basepath set to {}", s_basepath.string());
    LOG_DEBUG("gamepath set to {}", s_gamepath.string());
    LOG_DEBUG("userpath set to {}", s_userpath.string());

    std::filesystem::create_directories(s_userpath);

    auto mount_basepath_ok = PHYSFS_mount(s_basepath.string().c_str(), nullptr, false);
    vx::throw_if_not_fmt(mount_basepath_ok, "failed to mount {}: {}", s_basepath.string(), utils::physfs_error());

    if(s_gamepath.compare(s_basepath)) {
        auto mount_gamepath_ok = PHYSFS_mount(s_gamepath.string().c_str(), nullptr, false);
        vx::throw_if_not_fmt(mount_gamepath_ok, "failed to mount {}: {}", s_gamepath.string(), utils::physfs_error());
    }

    auto mount_userpath_ok = PHYSFS_mount(s_userpath.string().c_str(), nullptr, false);
    vx::throw_if_not_fmt(mount_userpath_ok, "failed to mount {}: {}", s_userpath.string(), utils::physfs_error());

    auto set_write_dir_ok = PHYSFS_setWriteDir(s_userpath.string().c_str());
    vx::throw_if_not_fmt(set_write_dir_ok, "failed to setwritedir {}: {}", s_userpath.string(), utils::physfs_error());

    cmd::add("version", &cmd::builtin::cmd_version);
    cmd::add("get", &config::cmd_get);
    cmd::add("set", &config::cmd_set);
}

void core::teardown(void)
{
    enet_deinitialize();

    auto physfs_deinit_ok = PHYSFS_deinit();
    vx::throw_if_not_fmt(physfs_deinit_ok, "failed to de-initialize physfs: {}", utils::physfs_error());
}
