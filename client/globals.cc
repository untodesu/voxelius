#include "client/pch.hh"

#include "client/globals.hh"

#include "core/config/map.hh"

config::Map globals::client_config;

SDL_Window* globals::window;
SDL_GPUDevice* globals::gpu_device;

SDL_GPUCommandBuffer* globals::gpu_commands_main;
SDL_GPUTexture* globals::gpu_swapchain;

int globals::width;
int globals::height;
float globals::aspect;

std::uint64_t globals::fixed_accumulator_us;
std::size_t globals::window_framecount;
std::uint64_t globals::window_frametime_us;
float globals::window_frametime;
float globals::window_frametime_avg;

std::size_t globals::num_draw_calls;
std::size_t globals::num_draw_vertices;
