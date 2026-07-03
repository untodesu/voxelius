#include "client/pch.hh"

#include "client/globals.hh"

#include "core/config/map.hh"

config::Map globals::client_config;

SDL_Window* globals::window = nullptr;

SDL_GPUDevice* globals::gpu_device = nullptr;
SDL_GPUTexture* globals::gpu_swapchain = nullptr;
SDL_GPUCommandBuffer* globals::gpu_commands_main = nullptr;

SDL_GPUTexture* globals::gpu_depth_stencil = nullptr;

std::size_t globals::num_draw_calls = 0;
std::size_t globals::num_draw_vertices = 0;

std::size_t globals::window_framecount = 0;
std::uint64_t globals::window_frametime_us = 0;
float globals::window_frametime = 0.0f;
float globals::window_frametime_avg = 0.0f;

unsigned int globals::gui_scale = 1;

ImFont* globals::font_default = nullptr;

std::uint64_t globals::fixed_accumulator_us = 0;
