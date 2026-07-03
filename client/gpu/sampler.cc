#include "client/pch.hh"

#include "client/gpu/sampler.hh"

#include "client/globals.hh"

gpu::Sampler gpu::Sampler::create(const SDL_GPUSamplerCreateInfo& info) noexcept
{
    auto handle = SDL_CreateGPUSampler(globals::gpu_device, &info);

    if(handle == nullptr) {
        LOG_WARNING("SDL_CreateGPUSampler failed: {}", SDL_GetError());
        return {};
    }

    return Sampler(globals::gpu_device, handle);
}

gpu::Sampler gpu::Sampler::create(SDL_GPUFilter filter, SDL_GPUSamplerMipmapMode mipmap_mode,
    SDL_GPUSamplerAddressMode address_mode) noexcept
{
    SDL_GPUSamplerCreateInfo info {};
    info.min_filter = filter;
    info.mag_filter = filter;
    info.mipmap_mode = mipmap_mode;
    info.address_mode_u = address_mode;
    info.address_mode_v = address_mode;
    info.address_mode_w = address_mode;

    return create(info);
}
