#include "client/pch.hh"

#include "client/head.hh"

#include "core/camera.hh"
#include "core/cmdline.hh"
#include "core/exception.hh"

#include "client/chunk_renderer.hh"
#include "client/globals.hh"

void head::init(void)
{
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

    chunk_renderer::init();

    SDL_ShowWindow(globals::window);
}

void head::init_late(void)
{
    // empty
}

void head::shutdown(void)
{
    chunk_renderer::shutdown();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DestroyContext(globals::gl_context);
}

bool head::prepare(void)
{
    int width, height;
    SDL_GetWindowSizeInPixels(globals::window, &width, &height);
    glViewport(0, 0, width, height);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    return true;
}

void head::render(void)
{
    chunk_renderer::render();
}

void head::present(void)
{
    ImGui::Render();

    auto draw_data = ImGui::GetDrawData();
    globals::num_draw_calls += draw_data->CmdLists.Size;
    globals::num_draw_vertices += draw_data->TotalVtxCount;

    ImGui_ImplOpenGL3_RenderDrawData(draw_data);

    SDL_GL_SwapWindow(globals::window);
}
