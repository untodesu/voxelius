// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: video.cc; Created: Tue Dec 30 2025 13:00:24
// Description: Video mode handling

#include "client/pch.hh"

#include "client/io/video.hh"

#include "core/config/boolean.hh"
#include "core/config/string.hh"

#include "core/io/cmdline.hh"
#include "core/io/config_map.hh"

#include "core/resource/image.hh"
#include "core/resource/resource.hh"

#include "core/version.hh"

#include "shared/splash.hh"

#include "client/gui/settings.hh"

#include "client/const.hh"
#include "client/globals.hh"

static glm::ivec2 last_windowed_size;
static config::Boolean enable_vsync(true);
static config::String current_mode("windowed");

static GLFWmonitor* fullscreen_monitor;
static std::vector<VideoMode> fullscreen_modes;

static void on_glfw_error(int error, const char* description)
{
    spdlog::error("video: GLFW error [{}]: {}", error, description);
}

static void on_glfw_framebuffer_size(GLFWwindow* window, int wide, int tall)
{
    globals::width = wide;
    globals::height = tall;
    globals::aspect = static_cast<float>(wide) / static_cast<float>(tall);

    if(nullptr == glfwGetWindowMonitor(window)) {
        last_windowed_size.x = wide;
        last_windowed_size.y = tall;
    }

    globals::dispatcher.trigger(FramebufferSizeEvent(wide, tall));
}

static void on_glfw_window_close(GLFWwindow* window)
{
    // We don't really have a good way to
    // pass "i want to quit" boolean to the main loop,
    // so instead we just raise an external interrupt signal
    // which handler latches an internal flag in the main loop
    std::raise(SIGINT);
}

static void on_window_focus_glfw(GLFWwindow* window, int focused)
{
    ImGui_ImplGlfw_WindowFocusCallback(window, focused);
}

void video::init(void)
{
    last_windowed_size.x = BASE_WIDTH;
    last_windowed_size.y = BASE_HEIGHT;

    globals::client_config.add_value("video.enable_vsync", enable_vsync);
    globals::client_config.add_value("video.current_mode", current_mode);

#ifdef __unix__
    // Wayland constantly throws random bullshit at me
    // when I'm dealing with pretty much anything cross-platform
    // on pretty much any kind of UNIX and Linux distribution
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
#endif

    glfwSetErrorCallback(&on_glfw_error);

    if(!glfwInit()) {
        spdlog::critical("glfw: initialize failed");
        std::terminate();
    }

    unsigned int monitor_index;
    auto monitor_arg = cmdline::get("monitor");
    auto monitor_check = std::from_chars(monitor_arg.data(), monitor_arg.data() + monitor_arg.size(), monitor_index);

    if(monitor_check.ec == std::errc()) {
        int monitor_count;
        const auto monitors = glfwGetMonitors(&monitor_count);

        if(monitor_index < static_cast<unsigned int>(monitor_count)) {
            fullscreen_monitor = monitors[monitor_index];
        }
        else {
            // If the user wants to, say run the game on a monitor
            // number 2 and there are only 2 monitors (remember, zero-based index)
            // it's a good idea to silently fall back to the primary monitor
            fullscreen_monitor = glfwGetPrimaryMonitor();
        }
    }
    else {
        fullscreen_monitor = glfwGetPrimaryMonitor();
    }

    int video_mode_count;
    const auto video_modes = glfwGetVideoModes(fullscreen_monitor, &video_mode_count);

    fullscreen_modes.clear();
    fullscreen_modes.reserve(video_mode_count);

    for(int i = 0; i < video_mode_count; ++i) {
        auto& mode = video_modes[i];

        // Only allow video modes that are at least as large as the base resolution
        // to be used by the video subsystem, otherwise we're going to end up with
        // some bizzare UI scaling issues because there is no UI scale less than 1.0
        if(mode.width >= BASE_WIDTH && mode.height >= BASE_HEIGHT) {
            fullscreen_modes.push_back(VideoMode(mode));
        }
    }

    // Setup GLFW window hints
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#if defined(__APPLE__)
    // Enable forward compatibility because Apple
    // just decided to be the autistic kid of the class
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    globals::window = glfwCreateWindow(BASE_WIDTH, BASE_HEIGHT, "Client", nullptr, nullptr);

    if(globals::window == nullptr) {
        spdlog::critical("glfw: window creation failed");
        std::terminate();
    }

    glfwSetWindowSizeLimits(globals::window, BASE_WIDTH, BASE_HEIGHT, GLFW_DONT_CARE, GLFW_DONT_CARE);
    glfwMakeContextCurrent(globals::window);
    glfwSwapInterval(1);

    glfwSetFramebufferSizeCallback(globals::window, &on_glfw_framebuffer_size);
    glfwSetWindowCloseCallback(globals::window, &on_glfw_window_close);
    glfwSetWindowFocusCallback(globals::window, &on_window_focus_glfw);

    if(auto image = resource::load<Image>("textures/gui/window_icon.png")) {
        GLFWimage icon_image;
        icon_image.width = image->size.x;
        icon_image.height = image->size.y;
        icon_image.pixels = reinterpret_cast<unsigned char*>(image->pixels);
        glfwSetWindowIcon(globals::window, 1, &icon_image);
    }

    settings::add_video_mode_select(0, settings_location::VIDEO, "video.current_mode");
    settings::add_checkbox(1, enable_vsync, settings_location::VIDEO, "video.enable_vsync", true);

    update_window_title();
}

void video::init_late(void)
{
    std::string mode_string(current_mode.get_value());

    if(0 == mode_string.compare("windowed")) {
        request_windowed(last_windowed_size.x, last_windowed_size.y);
        return;
    }

    int target_width;
    int target_height;
    int target_refresh_rate;

    if(3 == std::sscanf(mode_string.c_str(), "%d:%d:%d", &target_width, &target_height, &target_refresh_rate)) {
        request_fullscreen(target_width, target_height, target_refresh_rate);
        return;
    }

    int current_width;
    int current_height;
    glfwGetFramebufferSize(globals::window, &current_width, &current_height);
    on_glfw_framebuffer_size(globals::window, current_width, current_height);
}

void video::shutdown(void)
{
    glfwDestroyWindow(globals::window);
    glfwTerminate();
}

void video::update(void)
{
    glfwGetFramebufferSize(globals::window, &globals::width, &globals::height);
    globals::aspect = static_cast<float>(globals::width) / static_cast<float>(globals::height);
}

void video::update_late(void)
{
    glfwSwapInterval(enable_vsync.get_value() ? 1 : 0);
}

void video::query_current_mode(int& wide, int& tall) noexcept
{
    glfwGetFramebufferSize(globals::window, &wide, &tall);
}

void video::query_current_mode(int& wide, int& tall, bool& fullscreen) noexcept
{
    glfwGetFramebufferSize(globals::window, &wide, &tall);
    fullscreen = static_cast<bool>(glfwGetWindowMonitor(globals::window));
}

const std::vector<VideoMode>& video::query_fullscreen_modes(void) noexcept
{
    return fullscreen_modes;
}

void video::request_fullscreen(int wide, int tall, int rate) noexcept
{
    assert(wide >= BASE_WIDTH);
    assert(tall >= BASE_HEIGHT);

    std::size_t best_index = 0;
    auto best_score = std::numeric_limits<int>::max();

    for(std::size_t i = 0; i < fullscreen_modes.size(); ++i) {
        const auto& mode = fullscreen_modes[i];

        auto score_width = std::abs(mode.wide() - wide);
        auto score_height = std::abs(mode.tall() - tall);
        auto score_refresh_rate = std::abs(mode.rate() - rate);
        auto total_score = score_width + score_height + score_refresh_rate;

        if(total_score < best_score) {
            best_score = total_score;
            best_index = i;
        }
    }

    const auto& best_mode = fullscreen_modes[best_index];

    glfwSetWindowMonitor(globals::window, fullscreen_monitor, 0, 0, best_mode.wide(), best_mode.tall(), best_mode.rate());
    glfwSetWindowAttrib(globals::window, GLFW_AUTO_ICONIFY, GLFW_FALSE);
    glfwSetWindowAttrib(globals::window, GLFW_RESIZABLE, GLFW_FALSE);

    current_mode.set(std::format("{}:{}:{}", best_mode.wide(), best_mode.tall(), best_mode.rate()));

    spdlog::debug("video: set mode to: {}x{} ({} Hz)", best_mode.wide(), best_mode.tall(), best_mode.rate());
}

void video::request_windowed(int wide, int tall) noexcept
{
    glfwSetWindowMonitor(globals::window, nullptr, 0, 0, wide, tall, GLFW_DONT_CARE);
    glfwSetWindowAttrib(globals::window, GLFW_AUTO_ICONIFY, GLFW_FALSE);
    glfwSetWindowAttrib(globals::window, GLFW_RESIZABLE, GLFW_TRUE);

    int workarea_xpos, workarea_ypos;
    int workarea_width, workarea_height;
    glfwGetMonitorWorkarea(fullscreen_monitor, &workarea_xpos, &workarea_ypos, &workarea_width, &workarea_height);

    auto center_x = workarea_xpos + (workarea_width - wide) / 2;
    auto center_y = workarea_ypos + (workarea_height - tall) / 2;

    glfwSetWindowPos(globals::window, center_x, center_y);

    current_mode.set("windowed");

    spdlog::debug("video: set mode to: windowed {}x{}", wide, tall);
}

void video::request_windowed(void) noexcept
{
    request_windowed(last_windowed_size.x, last_windowed_size.y);
}

void video::update_window_title(void)
{
    auto title = std::format("Voxelius {}: {}", version::triplet, splash::get());
    glfwSetWindowTitle(globals::window, title.c_str());
}
