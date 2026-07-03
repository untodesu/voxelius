#include "client/pch.hh"

#include "client/gpu/shader.hh"

#include "client/globals.hh"

gpu::Shader gpu::Shader::create(const SDL_GPUShaderCreateInfo& info) noexcept
{
    auto handle = SDL_CreateGPUShader(globals::gpu_device, &info);

    if(handle == nullptr) {
        LOG_WARNING("SDL_CreateGPUShader failed: {}", SDL_GetError());
        return {};
    }

    return Shader(globals::gpu_device, handle);
}
