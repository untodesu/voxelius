// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: main.cc
// Description: Entry point

#include "client/pch.hh"

#include "core/io/cmdline.hh"
#include "core/io/config_map.hh"

#include "core/resource/image.hh"
#include "core/resource/resource.hh"

#include "core/utils/epoch.hh"

#include "core/threading.hh"
#include "core/version.hh"

#include "shared/game.hh"
#include "shared/splash.hh"

#include "client/io/gamepad.hh"
#include "client/io/keyboard.hh"
#include "client/io/mouse.hh"
#include "client/io/video.hh"

#include "client/resource/sound_effect.hh"
#include "client/resource/texture_gui.hh"

#include "client/const.hh"
#include "client/game.hh"
#include "client/globals.hh"

#if defined(_WIN32)
extern "C" __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
extern "C" __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
#endif

std::atomic_bool is_running;

static void GLAD_API_PTR on_opengl_message(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message,
    const void* param)
{
    spdlog::info("opengl: {}", reinterpret_cast<const char*>(message));
}

static void on_termination_signal(int)
{
    spdlog::warn("client: received termination signal");

    is_running = false;
}

int main(int argc, char** argv)
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

    spdlog::info("Voxelius Client {}", version::full);

    video::init();

    keyboard::init();
    mouse::init();
    gamepad::init();

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

    spdlog::info("opengl: version: {}", reinterpret_cast<const char*>(glGetString(GL_VERSION)));
    spdlog::info("opengl: renderer: {}", reinterpret_cast<const char*>(glGetString(GL_RENDERER)));

    Image::register_resource();
    TextureGUI::register_resource();
    SoundEffect::register_resource();

    glDisable(GL_MULTISAMPLE);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(globals::window, false);
    ImGui_ImplOpenGL3_Init(nullptr);

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
                spdlog::info("sound: {}", reinterpret_cast<const char*>(alcGetString(globals::sound_dev, ALC_DEVICE_SPECIFIER)));

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

    video::update_window_title();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    globals::fixed_frametime = 0.0f;
    globals::fixed_frametime_avg = 0.0f;
    globals::fixed_frametime_us = UINT64_MAX;
    globals::fixed_framecount = 0;

    globals::curtime = utils::unix_microseconds();

    globals::window_frametime = 0.0f;
    globals::window_frametime_avg = 0.0f;
    globals::window_frametime_us = 0;
    globals::window_framecount = 0;

    int vmode_width = DEFAULT_WIDTH;
    int vmode_height = DEFAULT_HEIGHT;

    if(auto vmode = cmdline::get_cstr("mode")) {
        std::sscanf(vmode, "%dx%d", &vmode_width, &vmode_height);
        vmode_height = glm::max(vmode_height, MIN_HEIGHT);
        vmode_width = glm::max(vmode_width, MIN_WIDTH);
    }

    glfwSetWindowSize(globals::window, vmode_width, vmode_height);

    client_game::init();

    threading::init();

    globals::client_config.load_file("client.conf");
    globals::client_config.load_cmdline();

    video::init_late();

    client_game::init_late();

    auto last_curtime = globals::curtime;

    while(!glfwWindowShouldClose(globals::window)) {
        globals::curtime = utils::unix_microseconds();

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

        video::update();

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

        video::update_late();

        gamepad::update_late();

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

        resource::soft_cleanup();

        threading::update();
    }

    client_game::shutdown();

    resource::hard_cleanup();

    spdlog::info("client: shutdown after {} frames", globals::window_framecount);
    spdlog::info("client: average framerate: {:.03f} FPS", 1.0f / globals::window_frametime_avg);
    spdlog::info("client: average frametime: {:.03f} ms", 1000.0f * globals::window_frametime_avg);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    if(globals::sound_ctx) {
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(globals::sound_ctx);
        alcCloseDevice(globals::sound_dev);
    }

    video::shutdown();

    globals::client_config.save_file("client.conf");

    threading::shutdown();

    shared_game::shutdown();

    return EXIT_SUCCESS;
}
