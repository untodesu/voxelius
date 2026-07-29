#include "client/pch.hh"

#include "client/head.hh"

#include "core/camera.hh"
#include "core/cmdline.hh"
#include "core/config/map.hh"
#include "core/config/ref.hh"
#include "core/exception.hh"

#include "client/fog.hh"
#include "client/game.hh"
#include "client/globals.hh"
#include "client/gui/settings.hh"
#include "client/world/chunk_renderer.hh"
#include "client/world/chunk_vbo.hh"
#include "client/world/outline.hh"

static GLuint s_world_fbo;
static GLuint s_world_texture;
static GLuint s_world_renderbuffer;

static config::Ref<int> s_pixel_size { 1 };
static config::Ref<unsigned> s_fog_model { 2 };

static int s_scaled_width;
static int s_scaled_height;

static void update_framebuffer(int new_width, int new_height)
{
    auto pixel_size = std::clamp<int>(s_pixel_size.value(), 1, 4);
    s_scaled_width = new_width / pixel_size;
    s_scaled_height = new_height / pixel_size;

    if(s_world_fbo == 0) {
        glGenFramebuffers(1, &s_world_fbo);
        glGenTextures(1, &s_world_texture);
        glGenRenderbuffers(1, &s_world_renderbuffer);
    }

    glBindTexture(GL_TEXTURE_2D, s_world_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, s_scaled_width, s_scaled_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindRenderbuffer(GL_RENDERBUFFER, s_world_renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, s_scaled_width, s_scaled_height);

    glBindFramebuffer(GL_FRAMEBUFFER, s_world_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, s_world_texture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, s_world_renderbuffer);

    auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    vx::throw_if_not(status == GL_FRAMEBUFFER_COMPLETE, "failed to update world FBO");
}

static void on_sdl_window_event(const SDL_WindowEvent& event)
{
    if(event.type == SDL_EVENT_WINDOW_RESIZED) {
        update_framebuffer(event.data1, event.data2);
        return;
    }
}

static void present_imgui(void)
{
}

void head::init(void)
{
    s_pixel_size.bind(globals::client_config, "head.pixel_size");
    s_fog_model.bind(globals::client_config, "head.fog_model");

    settings::slider<int>(2, settings_location::VIDEO, "head.pixel_size", 1, 4, true);
    settings::stepper<unsigned>(3, settings_location::VIDEO, "head.fog_model", 0, 2, 1, false);

    globals::gl_context = SDL_GL_CreateContext(globals::window);
    vx::throw_if_not_fmt(globals::gl_context, "SDL_GL_CreateContext failed: {}", SDL_GetError());

    SDL_GL_MakeCurrent(globals::window, globals::gl_context);

    auto glad_ok = gladLoadGL(reinterpret_cast<GLADloadfunc>(&SDL_GL_GetProcAddress));
    vx::throw_if_not(glad_ok, "failed to load OpenGL function pointers");

    LOG_INFO("GL_VERSION: {}", reinterpret_cast<const char*>(glGetString(GL_VERSION)));
    LOG_INFO("GL_RENDERER: {}", reinterpret_cast<const char*>(glGetString(GL_RENDERER)));

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    auto imgui_platform_ok = ImGui_ImplSDL3_InitForOpenGL(globals::window, globals::gl_context);
    vx::throw_if_not(imgui_platform_ok, "ImGui_ImplSDL3_InitForOpenGL failed");

    auto imgui_renderer_ok = ImGui_ImplOpenGL3_Init("#version 330 core");
    vx::throw_if_not(imgui_renderer_ok, "ImGui_ImplOpenGL3_Init failed");

    ImGui::GetIO().IniFilename = nullptr;

    chunk_vbo::init();
    chunk_renderer::init();
    outline::init();

    SDL_ShowWindow(globals::window);

    globals::dispatcher.sink<SDL_WindowEvent>().connect<&on_sdl_window_event>();
}

void head::init_late(void)
{
    // empty
}

void head::shutdown(void)
{
    outline::shutdown();
    chunk_renderer::shutdown();
    chunk_vbo::shutdown();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    glDeleteRenderbuffers(1, &s_world_renderbuffer);
    glDeleteTextures(1, &s_world_texture);
    glDeleteFramebuffers(1, &s_world_fbo);

    SDL_GL_DestroyContext(globals::gl_context);
}

bool head::prepare(void)
{
    ZoneScoped;

    int width, height;
    SDL_GetWindowSizeInPixels(globals::window, &width, &height);
    glViewport(0, 0, width, height);

    if(s_pixel_size.dirty()) {
        update_framebuffer(width, height);
    }

    glClearColor(0.0f, 0.0f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    return true;
}

void head::render(void)
{
    ZoneScoped;

    glBindFramebuffer(GL_FRAMEBUFFER, s_world_fbo);
    glViewport(0, 0, s_scaled_width, s_scaled_height);

    auto clear_r = fog::color.x();
    auto clear_g = fog::color.y();
    auto clear_b = fog::color.z();
    auto clear_a = 1.0f;

    glClearDepth(1.0f);
    glClearColor(clear_r, clear_g, clear_b, clear_a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    chunk_renderer::render();

    client_game::render();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, globals::width, globals::height);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, s_world_fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, s_scaled_width, s_scaled_height, 0, 0, globals::width, globals::height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void head::present(void)
{
    ZoneScoped;

    ImGui::Render();

    auto draw_data = ImGui::GetDrawData();
    globals::num_draw_calls += draw_data->CmdLists.Size;
    globals::num_draw_vertices += draw_data->TotalVtxCount;

    ImGui_ImplOpenGL3_RenderDrawData(draw_data);

    SDL_GL_SwapWindow(globals::window);
}
