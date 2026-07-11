#include "client/pch.hh"

#include "client/head.hh"

#include "core/camera.hh"
#include "core/cmdline.hh"
#include "core/exception.hh"

#include "client/chunk_renderer.hh"
#include "client/globals.hh"

static void create_depth_stencil(int width, int height)
{
    if(globals::gpu_depth_stencil) {
        SDL_ReleaseGPUTexture(globals::gpu_device, globals::gpu_depth_stencil);
    }

    SDL_GPUTextureCreateInfo info {};
    info.type = SDL_GPU_TEXTURETYPE_2D;
    info.format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT_S8_UINT;
    info.usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;
    info.width = static_cast<Uint32>(width);
    info.height = static_cast<Uint32>(height);
    info.layer_count_or_depth = 1;
    info.num_levels = 1;
    info.sample_count = SDL_GPU_SAMPLECOUNT_1;

    globals::gpu_depth_stencil = SDL_CreateGPUTexture(globals::gpu_device, &info);
    vx::throw_if_not_fmt(globals::gpu_depth_stencil, "SDL_CreateGPUTexture (depth/stencil) failed: {}", SDL_GetError());
}

static void on_window_resized(const SDL_WindowEvent& event)
{
    if(event.type == SDL_EVENT_WINDOW_RESIZED && event.windowID == SDL_GetWindowID(globals::window)) {
        create_depth_stencil(event.data1, event.data2);
    }
}

void head::init(void)
{
    auto debug_mode = cmdline::contains("gpu-debug");
    globals::gpu_device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, debug_mode, nullptr);
    vx::throw_if_not_fmt(globals::gpu_device, "SDL_CreateGPUDevice failed: {}", SDL_GetError());

    auto claimed = SDL_ClaimWindowForGPUDevice(globals::gpu_device, globals::window);
    vx::throw_if_not_fmt(claimed, "SDL_ClaimWindowForGPUDevice failed: {}", SDL_GetError());

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    auto imgui_platform_ok = ImGui_ImplSDL3_InitForSDLGPU(globals::window);
    vx::throw_if_not(imgui_platform_ok, "ImGui_ImplSDL3_InitForSDLGPU failed");

    ImGui_ImplSDLGPU3_InitInfo imgui_gpu_info {};
    imgui_gpu_info.Device = globals::gpu_device;
    imgui_gpu_info.ColorTargetFormat = SDL_GetGPUSwapchainTextureFormat(globals::gpu_device, globals::window);

    auto imgui_renderer_ok = ImGui_ImplSDLGPU3_Init(&imgui_gpu_info);
    vx::throw_if_not(imgui_renderer_ok, "ImGui_ImplSDLGPU3_Init failed");

    ImGui::GetIO().IniFilename = nullptr;

    int width, height;
    SDL_GetWindowSize(globals::window, &width, &height);
    create_depth_stencil(width, height);

    globals::dispatcher.sink<SDL_WindowEvent>().connect<&on_window_resized>();

    chunk_renderer::init();

    SDL_ShowWindow(globals::window);
}

void head::init_late(void)
{
    int width, height;
    SDL_GetWindowSize(globals::window, &width, &height);
    create_depth_stencil(width, height);
}

void head::shutdown(void)
{
    chunk_renderer::shutdown();

    SDL_ReleaseGPUTexture(globals::gpu_device, globals::gpu_depth_stencil);

    ImGui_ImplSDLGPU3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_ReleaseWindowFromGPUDevice(globals::gpu_device, globals::window);
    SDL_DestroyGPUDevice(globals::gpu_device);
}

bool head::prepare(void)
{
    globals::gpu_commands_main = SDL_AcquireGPUCommandBuffer(globals::gpu_device);
    vx::throw_if_not_fmt(globals::gpu_commands_main, "SDL_AcquireGPUCommandBuffer failed: {}", SDL_GetError());

    auto cmds = globals::gpu_commands_main;
    auto acquired = SDL_WaitAndAcquireGPUSwapchainTexture(cmds, globals::window, &globals::gpu_swapchain, nullptr, nullptr);
    vx::throw_if_not_fmt(acquired, "SDL_AcquireGPUSwapchainTexture failed: {}", SDL_GetError());

    if(globals::gpu_swapchain == nullptr) {
        SDL_SubmitGPUCommandBuffer(globals::gpu_commands_main);
        globals::gpu_commands_main = nullptr;
        return false;
    }

    ImGui_ImplSDLGPU3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    return true;
}

void head::render(void)
{
    auto copy_pass = SDL_BeginGPUCopyPass(globals::gpu_commands_main);
    vx::throw_if_not(copy_pass, "SDL_BeginGPUCopyPass returned nullptr");

    chunk_renderer::upload(copy_pass);

    SDL_EndGPUCopyPass(copy_pass);

    // TODO: wait for copy pass to finish maybe?

    SDL_GPUColorTargetInfo color_target_info {};
    color_target_info.texture = globals::gpu_swapchain;
    color_target_info.load_op = SDL_GPU_LOADOP_CLEAR;
    color_target_info.store_op = SDL_GPU_STOREOP_STORE;
    color_target_info.clear_color.r = 0.0f;
    color_target_info.clear_color.g = 0.0f;
    color_target_info.clear_color.b = 0.0f;
    color_target_info.clear_color.a = 1.0f;

    SDL_GPUDepthStencilTargetInfo depth_target_info {};
    depth_target_info.texture = globals::gpu_depth_stencil;
    depth_target_info.clear_depth = 1.0f;
    depth_target_info.clear_stencil = 0;
    depth_target_info.load_op = SDL_GPU_LOADOP_CLEAR;
    depth_target_info.store_op = SDL_GPU_STOREOP_STORE;
    depth_target_info.stencil_load_op = SDL_GPU_LOADOP_CLEAR;
    depth_target_info.stencil_store_op = SDL_GPU_STOREOP_STORE;

    auto render_pass = SDL_BeginGPURenderPass(globals::gpu_commands_main, &color_target_info, 1, &depth_target_info);
    vx::throw_if_not(render_pass, "SDL_BeginGPURenderPass returned nullptr");

    chunk_renderer::render(render_pass);

    SDL_EndGPURenderPass(render_pass);
}

void head::present(void)
{
    ImGui::Render();

    auto draw_data = ImGui::GetDrawData();

    globals::num_draw_calls += draw_data->CmdLists.Size;
    globals::num_draw_vertices += draw_data->TotalVtxCount;

    ImGui_ImplSDLGPU3_PrepareDrawData(draw_data, globals::gpu_commands_main);

    SDL_GPUColorTargetInfo color_target_info {};
    color_target_info.texture = globals::gpu_swapchain;
    color_target_info.load_op = SDL_GPU_LOADOP_LOAD; // draw over whatever head::render() already drew
    color_target_info.store_op = SDL_GPU_STOREOP_STORE;

    auto render_pass = SDL_BeginGPURenderPass(globals::gpu_commands_main, &color_target_info, 1, nullptr);
    vx::throw_if_not(render_pass, "SDL_BeginGPURenderPass returned nullptr");

    ImGui_ImplSDLGPU3_RenderDrawData(draw_data, globals::gpu_commands_main, render_pass);

    SDL_EndGPURenderPass(render_pass);

    SDL_SubmitGPUCommandBuffer(globals::gpu_commands_main);

    globals::gpu_commands_main = nullptr;
    globals::gpu_swapchain = nullptr;
}
