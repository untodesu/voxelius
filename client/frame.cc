#include "client/pch.hh"

#include "client/frame.hh"

#include "core/exception.hh"

#include "client/globals.hh"

bool frame::prepare(void)
{
    globals::gpu_commands_main = SDL_AcquireGPUCommandBuffer(globals::gpu_device);
    vx::throw_if_not_fmt(globals::gpu_commands_main, "SDL_AcquireGPUCommandBuffer failed: {}", SDL_GetError());

    auto swapchain_acquired = SDL_WaitAndAcquireGPUSwapchainTexture(globals::gpu_commands_main, globals::window, &globals::gpu_swapchain,
        nullptr, nullptr);
    vx::throw_if_not_fmt(swapchain_acquired, "SDL_AcquireGPUSwapchainTexture failed: {}", SDL_GetError());

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

void frame::present(void)
{
    ImGui::Render();

    auto draw_data = ImGui::GetDrawData();

    globals::num_draw_calls += draw_data->CmdLists.Size;
    globals::num_draw_vertices += draw_data->TotalVtxCount;

    ImGui_ImplSDLGPU3_PrepareDrawData(draw_data, globals::gpu_commands_main);

    SDL_GPUColorTargetInfo color_target_info {};
    color_target_info.texture = globals::gpu_swapchain;
    color_target_info.load_op = SDL_GPU_LOADOP_CLEAR;
    color_target_info.store_op = SDL_GPU_STOREOP_STORE;
    color_target_info.clear_color.r = 0.0f;
    color_target_info.clear_color.g = 0.0f;
    color_target_info.clear_color.b = 0.5f;
    color_target_info.clear_color.a = 1.0f;

    auto render_pass = SDL_BeginGPURenderPass(globals::gpu_commands_main, &color_target_info, 1, nullptr);
    vx::throw_if_not(render_pass != nullptr, "SDL_BeginGPURenderPass returned nullptr");

    ImGui_ImplSDLGPU3_RenderDrawData(draw_data, globals::gpu_commands_main, render_pass);

    SDL_EndGPURenderPass(render_pass);

    SDL_SubmitGPUCommandBuffer(globals::gpu_commands_main);

    globals::gpu_commands_main = nullptr;
    globals::gpu_swapchain = nullptr;
}
