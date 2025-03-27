#include "client/pch.hh"

#include "core/binfile.hh"
#include "core/cmdline.hh"
#include "core/config.hh"
#include "core/epoch.hh"
#include "core/feature.hh"
#include "core/image.hh"
#include "core/resource.hh"
#include "core/version.hh"

#include "shared/game.hh"
#include "shared/splash.hh"
#include "shared/threading.hh"

#include "client/const.hh"
#include "client/game.hh"
#include "client/glfw.hh"
#include "client/globals.hh"
#include "client/sound_effect.hh"
#include "client/texture_gui.hh"
#include "client/window_title.hh"

#if defined(_WIN32)
extern "C" __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
extern "C" __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
#endif

static void on_glfw_error(int code, const char *message)
{
    spdlog::error("glfw: {}", message);
}

static void on_glfw_char(GLFWwindow *window, unsigned int codepoint)
{
    ImGui_ImplGlfw_CharCallback(window, codepoint);
}

static void on_glfw_cursor_enter(GLFWwindow *window, int entered)
{
    ImGui_ImplGlfw_CursorEnterCallback(window, entered);
}

static void on_glfw_cursor_pos(GLFWwindow *window, double xpos, double ypos)
{
    GlfwCursorPosEvent event;
    event.pos.x = static_cast<float>(xpos);
    event.pos.y = static_cast<float>(ypos);
    globals::dispatcher.trigger(event);

    ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);
}

static void on_glfw_framebuffer_size(GLFWwindow *window, int width, int height)
{
    if(glfwGetWindowAttrib(window, GLFW_ICONIFIED)) {
        // Don't do anything if the window was just
        // iconified (minimized); as it turns out minimized
        // windows on WIN32 seem to be forced into 0x0
        return;
    }
    
    globals::width = width;
    globals::height = height;
    globals::aspect = static_cast<float>(width) / static_cast<float>(height);

    GlfwFramebufferSizeEvent fb_event;
    fb_event.size.x = globals::width;
    fb_event.size.y = globals::height;
    fb_event.aspect = globals::aspect;
    globals::dispatcher.trigger(fb_event);
}

static void on_glfw_key(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    GlfwKeyEvent event;
    event.key = key;
    event.scancode = scancode;
    event.action = action;
    event.mods = mods;
    globals::dispatcher.trigger(event);

    ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
}

static void on_glfw_joystick(int joystick_id, int event_type)
{
    GlfwJoystickEvent event;
    event.joystick_id = joystick_id;
    event.event_type = event_type;
    globals::dispatcher.trigger(event);
}

static void on_glfw_monitor_event(GLFWmonitor *monitor, int event)
{
    ImGui_ImplGlfw_MonitorCallback(monitor, event);
}

static void on_glfw_mouse_button(GLFWwindow *window, int button, int action, int mods)
{
    GlfwMouseButtonEvent event;
    event.button = button;
    event.action = action;
    event.mods = mods;
    globals::dispatcher.trigger(event);

    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
}

static void on_glfw_scroll(GLFWwindow *window, double dx, double dy)
{
    GlfwScrollEvent event;
    event.dx = static_cast<float>(dx);
    event.dy = static_cast<float>(dy);
    globals::dispatcher.trigger(event);

    ImGui_ImplGlfw_ScrollCallback(window, dx, dy);
}

static void on_glfw_window_focus(GLFWwindow *window, int focused)
{
    ImGui_ImplGlfw_WindowFocusCallback(window, focused);
}

static void GLAD_API_PTR on_opengl_message(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *param)
{
    spdlog::info("opengl: {}", reinterpret_cast<const char *>(message));
}

static void on_termination_signal(int)
{
    spdlog::warn("client: received termination signal");
    glfwSetWindowShouldClose(globals::window, true);
}

int main(int argc, char **argv)
{
    cmdline::create(argc, argv);

#if defined(_WIN32)
#if defined(NDEBUG)
    if(GetConsoleWindow() && !cmdline::contains("debug")) {
        // Hide the console window on release builds
        // unless explicitly specified to preserve it instead
        FreeConsole();
    }
#else
    if(GetConsoleWindow() && cmdline::contains("nodebug")) {
        // Hide the console window on debug builds when
        // explicitly specified by the user to hide it
        FreeConsole();
    }
#endif
#endif

    shared_game::init(argc, argv);

    spdlog::info("Voxelius Client {}", PROJECT_VERSION_STRING);

    glfwSetErrorCallback(&on_glfw_error);

#if defined(__unix__)
    // Wayland constantly throws random bullshit at me
    // when I'm dealing with pretty much anything cross-platform
    // on pretty much any kind of UNIX and Linux distribution
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
#endif

    if(!glfwInit()) {
        spdlog::critical("glfw: init failed");
        std::terminate();
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_SAMPLES, 0);

    globals::window = glfwCreateWindow(DEFAULT_WIDTH, DEFAULT_HEIGHT, "Client", nullptr, nullptr);

    if(!globals::window) {
        spdlog::critical("glfw: failed to open a window");
        std::terminate();
    }

    std::signal(SIGINT, &on_termination_signal);
    std::signal(SIGTERM, &on_termination_signal);

    glfwMakeContextCurrent(globals::window);
    glfwSwapInterval(1);

    if(!gladLoadGL(&glfwGetProcAddress)) {
        spdlog::critical("glad: failed to load function pointers");
        std::terminate();
    }

    if(GLAD_GL_KHR_debug) {
        if(!cmdline::contains("nodebug")) {
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallback(&on_opengl_message, nullptr);

            // NVIDIA drivers tend to spam quote-unquote "useful"
            // information about buffer usage into the debug callback
            static const std::uint32_t ignore_nvidia_131185 = 131185;
            glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_OTHER, GL_DONT_CARE, 1, &ignore_nvidia_131185, GL_FALSE);
        }
        else {
            spdlog::warn("glad: nodebug command line parameter found");
            spdlog::warn("glad: OpenGL errors will not be logged");
        }
    }
    else {
        spdlog::warn("glad: KHR_debug extension not supported");
        spdlog::warn("glad: OpenGL errors will not be logged");
    }

    spdlog::info("opengl: version: {}", reinterpret_cast<const char *>(glGetString(GL_VERSION)));
    spdlog::info("opengl: renderer: {}", reinterpret_cast<const char *>(glGetString(GL_RENDERER)));

    glDisable(GL_MULTISAMPLE);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(globals::window, false);
    ImGui_ImplOpenGL3_Init(nullptr);

    // The UI is scaled against a resolution defined by BASE_WIDTH and BASE_HEIGHT
    // constants. However, UI scale of 1 doesn't look that good, so the window size is
    // limited to a resolution that allows at least UI scale of 2 and is defined by MIN_WIDTH and MIN_HEIGHT.
    glfwSetWindowSizeLimits(globals::window, MIN_WIDTH, MIN_HEIGHT, GLFW_DONT_CARE, GLFW_DONT_CARE);
    
    glfwSetCharCallback(globals::window, &on_glfw_char);
    glfwSetCursorEnterCallback(globals::window, &on_glfw_cursor_enter);
    glfwSetCursorPosCallback(globals::window, &on_glfw_cursor_pos);
    glfwSetFramebufferSizeCallback(globals::window, &on_glfw_framebuffer_size);
    glfwSetKeyCallback(globals::window, &on_glfw_key);
    glfwSetMouseButtonCallback(globals::window, &on_glfw_mouse_button);
    glfwSetScrollCallback(globals::window, &on_glfw_scroll);
    glfwSetWindowFocusCallback(globals::window, &on_glfw_window_focus);

    glfwSetJoystickCallback(&on_glfw_joystick);
    glfwSetMonitorCallback(&on_glfw_monitor_event);

    if(auto image = resource::load<Image>("textures/gui/window_icon.png")) {
        GLFWimage icon_image;
        icon_image.width = image->size.x;
        icon_image.height = image->size.y;
        icon_image.pixels = reinterpret_cast<unsigned char *>(image->pixels);
        glfwSetWindowIcon(globals::window, 1, &icon_image);
    }

    if(cmdline::contains("nosound")) {
        spdlog::warn("client: sound disabled [per command line]");
        globals::sound_dev = nullptr;
        globals::sound_ctx = nullptr;
    }
    else {
        if(!saladLoadALdefault()) {
            spdlog::warn("client: sound disabled [openal loading failed]");
            globals::sound_dev = nullptr;
            globals::sound_ctx = nullptr;
        }
        else {
            globals::sound_dev = alcOpenDevice(nullptr);

            if(globals::sound_dev == nullptr) {
                spdlog::warn("client: sound disabled [no device]");
                globals::sound_ctx = nullptr;
            }
            else {
                spdlog::info("sound: {}", reinterpret_cast<const char *>(alcGetString(globals::sound_dev, ALC_DEVICE_SPECIFIER)));

                globals::sound_ctx = alcCreateContext(globals::sound_dev, nullptr);

                if(globals::sound_ctx == nullptr) {
                    spdlog::warn("client: sound disabled [context creation failed]");
                    alcCloseDevice(globals::sound_dev);
                    globals::sound_dev = nullptr;
                }
                else {
                    alcMakeContextCurrent(globals::sound_ctx);
                }
            }
        }
    }

    splash::init_client();

    window_title::update();

    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    globals::fixed_frametime = 0.0f;
    globals::fixed_frametime_avg = 0.0f;
    globals::fixed_frametime_us = UINT64_MAX;
    globals::fixed_framecount = 0;

    globals::curtime = epoch::microseconds();

    globals::window_frametime = 0.0f;
    globals::window_frametime_avg = 0.0f;
    globals::window_frametime_us = 0;
    globals::window_framecount = 0;

    int vmode_width = DEFAULT_WIDTH;
    int vmode_height = DEFAULT_HEIGHT;

    if(auto vmode = cmdline::get("mode")) {
        std::sscanf(vmode, "%dx%d", &vmode_width, &vmode_height);
        vmode_height = cxpr::max(vmode_height, MIN_HEIGHT);
        vmode_width = cxpr::max(vmode_width, MIN_WIDTH);
    }

    glfwSetWindowSize(globals::window, vmode_width, vmode_height);

    client_game::init();

    int wwidth, wheight;
    glfwGetFramebufferSize(globals::window, &wwidth, &wheight);
    on_glfw_framebuffer_size(globals::window, wwidth, wheight);

    threading::init();

    globals::client_config.load_file("client.conf");
    globals::client_config.load_cmdline();

    client_game::init_late();

    auto last_curtime = globals::curtime;

    while(!glfwWindowShouldClose(globals::window)) {
        globals::curtime = epoch::microseconds();

        globals::window_frametime_us = globals::curtime - last_curtime;
        globals::window_frametime = static_cast<float>(globals::window_frametime_us) / 1000000.0f;
        globals::window_frametime_avg += globals::window_frametime;
        globals::window_frametime_avg *= 0.5f;

        if(globals::fixed_frametime_us == UINT64_MAX) {
            globals::fixed_framecount = 0;
            globals::fixed_accumulator = 0;
        }
        else {
            globals::fixed_accumulator += globals::window_frametime_us;
            globals::fixed_framecount = globals::fixed_accumulator / globals::fixed_frametime_us;
            globals::fixed_accumulator %= globals::fixed_frametime_us;
        }
    
        globals::num_drawcalls = 0;
        globals::num_triangles = 0;

        last_curtime = globals::curtime;

        for(std::uint64_t i = 0; i < globals::fixed_framecount; ++i)
            client_game::fixed_update();
        client_game::update();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    
        glDisable(GL_BLEND);
    
        glDisable(GL_DEPTH_TEST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, globals::width, globals::height);
    
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    
        // Make sure there is no stray program object
        // being bound to the context. Usually third-party
        // overlay software (such as RivaTuner) injects itself
        // into the rendering loop and binds internal objects,
        // which creates an incomprehensible visual mess
        glUseProgram(0);

        client_game::render();

        client_game::layout();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(globals::window);

        for(std::uint64_t i = 0; i < globals::fixed_framecount; ++i)
            client_game::fixed_update_late();
        client_game::update_late();

        glfwPollEvents();

        // EnTT provides two ways of dispatching events:
        // queued and immediate. When glfwPollEvents() is
        // called, immediate events are triggered across
        // the application, whilst queued ones are triggered
        // later by calling entt::dispatcher::update()
        globals::dispatcher.update();

        globals::window_framecount += 1;

        resource::soft_cleanup<BinFile>();
        resource::soft_cleanup<Image>();
        
        resource::soft_cleanup<SoundEffect>();
        resource::soft_cleanup<TextureGUI>();

        threading::update();
    }

    client_game::deinit();
    
    resource::hard_cleanup<BinFile>();
    resource::hard_cleanup<Image>();

    resource::hard_cleanup<SoundEffect>();
    resource::hard_cleanup<TextureGUI>();
    
    spdlog::info("client: shutdown after {} frames", globals::window_framecount);
    spdlog::info("client: average framerate: {:.03f} FPS", 1.0f / globals::window_frametime_avg);
    spdlog::info("client: average frametime: {:.03f} ms", 1000.0f * globals::window_frametime_avg);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    if(globals::sound_ctx){
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(globals::sound_ctx);
        alcCloseDevice(globals::sound_dev);
    }

    glfwDestroyWindow(globals::window);
    glfwTerminate();

    globals::client_config.save_file("client.conf");

    threading::deinit();

    shared_game::deinit();

    return EXIT_SUCCESS;
}
