#include "client/pch.hh"

#include "client/video.hh"

#include "core/cmdline.hh"
#include "core/config/map.hh"
#include "core/config/ref.hh"
#include "core/exception.hh"
#include "core/version.hh"

#include "shared/splash.hh"

#include "client/constant.hh"
#include "client/globals.hh"
#include "client/settings.hh"

static Eigen::Vector2i s_last_windowed_size;
static config::Ref<bool> s_enable_vsync { true };
static config::Ref<std::string> s_current_mode { "windowed" };

static SDL_DisplayID s_display;
static SDL_GPUPresentMode s_unlocked_present_mode;
static std::vector<SDL_DisplayMode> s_fullscreen_modes;

static void on_sdl_window_event(const SDL_WindowEvent& event)
{
    if(event.type != SDL_EVENT_WINDOW_RESIZED) {
        return;
    }

    if(event.windowID != SDL_GetWindowID(globals::window)) {
        return;
    }

    globals::width = event.data1;
    globals::height = event.data2;
    globals::aspect = static_cast<float>(event.data1) / static_cast<float>(event.data2);

    if(!SDL_GetWindowFullscreenMode(globals::window)) {
        s_last_windowed_size.x() = event.data1;
        s_last_windowed_size.y() = event.data2;
    }
}

static void pick_display(void)
{
    s_display = SDL_GetPrimaryDisplay();

    auto monitor_arg = cmdline::value("monitor");

    if(!monitor_arg.has_value()) {
        return;
    }

    unsigned monitor_index;
    auto check = std::from_chars(monitor_arg->data(), monitor_arg->data() + monitor_arg->size(), monitor_index);

    if(check.ec == std::errc {}) {
        int display_count;
        auto displays = SDL_GetDisplays(&display_count);

        if(monitor_index < static_cast<unsigned>(display_count)) {
            s_display = displays[monitor_index];
        }

        SDL_free(displays);
    }
}

static void cache_fullscreen_modes(void)
{
    int mode_count;
    auto modes = SDL_GetFullscreenDisplayModes(s_display, &mode_count);

    s_fullscreen_modes.clear();
    s_fullscreen_modes.reserve(mode_count);

    for(int i = 0; i < mode_count; ++i) {
        const auto& mode = *modes[i];

        // Only allow video modes that are at least as large as the base
        // resolution, otherwise the UI has nowhere sane to scale down to
        if(mode.w >= constant::BASE_WIDTH && mode.h >= constant::BASE_HEIGHT) {
            s_fullscreen_modes.push_back(mode);
        }
    }

    SDL_free(modes);

    std::sort(s_fullscreen_modes.begin(), s_fullscreen_modes.end(), [](const SDL_DisplayMode& a, const SDL_DisplayMode& b) {
        if(a.w == b.w && a.h == b.h)
            return a.refresh_rate < b.refresh_rate;
        if(a.w == b.w)
            return a.h < b.h;
        return a.w < b.w;
    });
}

static void update_present_mode(void)
{
    if(s_enable_vsync.value()) {
        SDL_GL_SetSwapInterval(1);
    }
    else {
        SDL_GL_SetSwapInterval(0);
    }
}

static void on_sdl_key(const SDL_KeyboardEvent& event)
{
    if(event.type == SDL_EVENT_KEY_DOWN) {
        if(event.key == SDLK_F1) {
            globals::client_config.set_value<bool>("video.enable_vsync", !s_enable_vsync.value());
        }

        if(event.key == SDLK_F2) {
            s_enable_vsync.set_value(!s_enable_vsync.value());
        }
    }
}

void video::init(void)
{
    s_last_windowed_size = Eigen::Vector2i(constant::BASE_WIDTH, constant::BASE_HEIGHT);

    s_enable_vsync.bind(globals::client_config, "video.enable_vsync");
    s_current_mode.bind(globals::client_config, "video.current_mode");

    vx::throw_if_not_fmt(SDL_InitSubSystem(SDL_INIT_VIDEO), "SDL_InitSubSystem for video failed: {}", SDL_GetError());

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);  // FIXME: use an actual framebuffer
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8); // FIXME: use an actual framebuffer

    pick_display();
    cache_fullscreen_modes();

    settings::video_mode(0, settings_location::VIDEO, "video.current_mode", false);
    settings::checkbox(1, settings_location::VIDEO, "video.enable_vsync", true);

    globals::window = SDL_CreateWindow("client", constant::BASE_WIDTH, constant::BASE_HEIGHT,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_OPENGL);
    vx::throw_if_not_fmt(globals::window, "SDL_CreateWindow failed: {}", SDL_GetError());

    SDL_SetWindowMinimumSize(globals::window, constant::BASE_WIDTH, constant::BASE_HEIGHT);

    globals::dispatcher.sink<SDL_WindowEvent>().connect<&on_sdl_window_event>();
    globals::dispatcher.sink<SDL_KeyboardEvent>().connect<&on_sdl_key>();

    update_window_title();
}

void video::init_late(void)
{
    update_present_mode();

    const std::string& mode = s_current_mode.value();

    if(mode == "windowed") {
        video::request_windowed(s_last_windowed_size.x(), s_last_windowed_size.y());
    }
    else {
        int target_width;
        int target_height;
        int target_rate;

        if(3 == std::sscanf(mode.c_str(), "%d:%d:%d", &target_width, &target_height, &target_rate)) {
            video::request_fullscreen(target_width, target_height, target_rate);
        }
    }

    int width;
    int height;
    SDL_GetWindowSizeInPixels(globals::window, &width, &height);

    globals::width = width;
    globals::height = height;
    globals::aspect = static_cast<float>(width) / static_cast<float>(height);
}

void video::shutdown(void)
{
    SDL_DestroyWindow(globals::window);
}

void video::update(void)
{
    // empty
}

void video::update_late(void)
{
    if(s_enable_vsync.dirty()) {
        update_present_mode();
    }
}

void video::query_current_mode(int& width, int& height)
{
    width = globals::width;
    height = globals::height;
}

void video::query_current_mode(int& width, int& height, bool& fullscreen)
{
    width = globals::width;
    height = globals::height;
    fullscreen = static_cast<bool>(SDL_GetWindowFullscreenMode(globals::window));
}

void video::query_current_mode(int& width, int& height, int& rate, bool& fullscreen)
{
    width = globals::width;
    height = globals::height;

    auto mode = SDL_GetWindowFullscreenMode(globals::window);
    rate = mode ? static_cast<int>(mode->refresh_rate) : 0;
    fullscreen = static_cast<bool>(mode);
}

const std::vector<SDL_DisplayMode>& video::query_fullscreen_modes(void)
{
    return s_fullscreen_modes;
}

void video::request_fullscreen(int width, int height, int rate)
{
    assert(width >= constant::BASE_WIDTH);
    assert(height >= constant::BASE_HEIGHT);

    std::size_t best_index = 0;
    auto best_score = std::numeric_limits<int>::max();

    for(std::size_t i = 0; i < s_fullscreen_modes.size(); ++i) {
        const auto& mode = s_fullscreen_modes[i];

        auto score_width = std::abs(mode.w - width);
        auto score_height = std::abs(mode.h - height);
        auto score_rate = std::abs(static_cast<int>(mode.refresh_rate) - rate);
        auto total_score = score_width + score_height + score_rate;

        if(total_score < best_score) {
            best_score = total_score;
            best_index = i;
        }
    }

    if(s_fullscreen_modes.empty()) {
        return;
    }

    const auto& best_mode = s_fullscreen_modes[best_index];

    SDL_SetWindowFullscreenMode(globals::window, &best_mode);
    SDL_SetWindowFullscreen(globals::window, true);

    s_current_mode.set_value(std::format("{}:{}:{}", best_mode.w, best_mode.h, static_cast<int>(best_mode.refresh_rate)));

    LOG_DEBUG("set mode to: {}x{} ({} Hz)", best_mode.w, best_mode.h, static_cast<int>(best_mode.refresh_rate));
}

void video::request_windowed(int width, int height)
{
    SDL_SetWindowFullscreen(globals::window, false);
    SDL_SetWindowSize(globals::window, width, height);

    SDL_Rect usable_bounds {};
    SDL_GetDisplayUsableBounds(s_display, &usable_bounds);

    auto center_x = usable_bounds.x + (usable_bounds.w - width) / 2;
    auto center_y = usable_bounds.y + (usable_bounds.h - height) / 2;

    SDL_SetWindowPosition(globals::window, center_x, center_y);

    s_current_mode.set_value("windowed");

    LOG_DEBUG("set mode to: windowed {}x{}", width, height);
}

void video::request_windowed(void)
{
    video::request_windowed(s_last_windowed_size.x(), s_last_windowed_size.y());
}

void video::update_window_title(void)
{
    auto title = std::format("Voxelius {}: {}", version::full, splash::get());

    SDL_SetWindowTitle(globals::window, title.c_str());
}
