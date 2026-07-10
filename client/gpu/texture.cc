#include "client/pch.hh"

#include "client/gpu/texture.hh"

#include "core/exception.hh"

#include "client/globals.hh"

static Uint32 resolve_mip_levels(int num_levels, Uint32 width, Uint32 height) noexcept
{
    if(num_levels >= 0) {
        return static_cast<Uint32>(num_levels);
    }

    auto largest = std::max(width, height);
    return static_cast<Uint32>(std::bit_width(largest));
}

gpu::Texture gpu::Texture::create(const SDL_GPUTextureCreateInfo& info) noexcept
{
    auto handle = SDL_CreateGPUTexture(globals::gpu_device, &info);

    if(handle == nullptr) {
        LOG_WARNING("SDL_CreateGPUTexture failed: {}", SDL_GetError());
        return {};
    }

    return Texture(globals::gpu_device, handle, info.width, info.height, info.format);
}

gpu::Texture gpu::Texture::create_2D(Uint32 width, Uint32 height, SDL_GPUTextureFormat format, SDL_GPUTextureUsageFlags usage,
    int num_levels) noexcept
{
    SDL_GPUTextureCreateInfo info {};
    info.type = SDL_GPU_TEXTURETYPE_2D;
    info.format = format;
    info.usage = usage;
    info.width = width;
    info.height = height;
    info.layer_count_or_depth = 1;
    info.num_levels = resolve_mip_levels(num_levels, width, height);
    info.sample_count = SDL_GPU_SAMPLECOUNT_1;

    return create(info);
}

gpu::Texture gpu::Texture::create_cube(Uint32 size, SDL_GPUTextureFormat format, SDL_GPUTextureUsageFlags usage, int num_levels) noexcept
{
    SDL_GPUTextureCreateInfo info {};
    info.type = SDL_GPU_TEXTURETYPE_CUBE;
    info.format = format;
    info.usage = usage;
    info.width = size;
    info.height = size;
    info.layer_count_or_depth = 6;
    info.num_levels = resolve_mip_levels(num_levels, size, size);
    info.sample_count = SDL_GPU_SAMPLECOUNT_1;

    return create(info);
}

gpu::Texture gpu::Texture::create_array(Uint32 width, Uint32 height, Uint32 layers, SDL_GPUTextureFormat format,
    SDL_GPUTextureUsageFlags usage, int num_levels) noexcept
{
    SDL_GPUTextureCreateInfo info {};
    info.type = SDL_GPU_TEXTURETYPE_2D_ARRAY;
    info.format = format;
    info.usage = usage;
    info.width = width;
    info.height = height;
    info.layer_count_or_depth = layers;
    info.num_levels = resolve_mip_levels(num_levels, width, height);
    info.sample_count = SDL_GPU_SAMPLECOUNT_1;

    return create(info);
}

void gpu::Texture::write_streamed(SDL_GPUCopyPass* copy_pass, std::span<const std::byte> data, Uint32 layer, Uint32 mip_level, Uint32 x,
    Uint32 y, Uint32 width, Uint32 height)
{
    SDL_GPUTransferBufferCreateInfo transfer_info {};
    transfer_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    transfer_info.size = static_cast<Uint32>(data.size());

    auto tbuf = SDL_CreateGPUTransferBuffer(m_device, &transfer_info);
    vx::throw_if_fmt(tbuf == nullptr, "SDL_CreateGPUTransferBuffer failed: {}", SDL_GetError());

    auto mapped = SDL_MapGPUTransferBuffer(m_device, tbuf, true);
    vx::throw_if_fmt(mapped == nullptr, "SDL_MapGPUTransferBuffer failed: {}", SDL_GetError());

    std::memcpy(mapped, data.data(), data.size());

    SDL_UnmapGPUTransferBuffer(m_device, tbuf);

    SDL_GPUTextureTransferInfo src_info {};
    src_info.transfer_buffer = tbuf;
    src_info.offset = 0;

    SDL_GPUTextureRegion dst_region {};
    dst_region.texture = m_handle;
    dst_region.mip_level = mip_level;
    dst_region.layer = layer;
    dst_region.x = x;
    dst_region.y = y;
    dst_region.w = width ? width : m_width;
    dst_region.h = height ? height : m_height;
    dst_region.d = 1;

    SDL_UploadToGPUTexture(copy_pass, &src_info, &dst_region, false);

    SDL_ReleaseGPUTransferBuffer(m_device, tbuf);
}

void gpu::Texture::write_blocking(std::span<const std::byte> data, Uint32 layer, Uint32 mip_level, Uint32 x, Uint32 y, Uint32 width,
    Uint32 height)
{
    auto cmds = SDL_AcquireGPUCommandBuffer(m_device);
    vx::throw_if_fmt(cmds == nullptr, "SDL_AcquireGPUCommandBuffer failed: {}", SDL_GetError());

    auto pass = SDL_BeginGPUCopyPass(cmds);
    vx::throw_if(pass == nullptr, "SDL_BeginGPUCopyPass returned null!");

    write_streamed(pass, data, layer, mip_level, x, y, width, height);

    SDL_EndGPUCopyPass(pass);

    auto fence = SDL_SubmitGPUCommandBufferAndAcquireFence(cmds);
    SDL_WaitForGPUFences(m_device, true, &fence, 1);
    SDL_ReleaseGPUFence(m_device, fence);
}

void gpu::Texture::generate_mipmaps_blocking(void) const
{
    auto cmds = SDL_AcquireGPUCommandBuffer(m_device);
    vx::throw_if_fmt(cmds == nullptr, "SDL_AcquireGPUCommandBuffer failed: {}", SDL_GetError());

    SDL_GenerateMipmapsForGPUTexture(cmds, m_handle);

    auto fence = SDL_SubmitGPUCommandBufferAndAcquireFence(cmds);
    SDL_WaitForGPUFences(m_device, true, &fence, 1);
    SDL_ReleaseGPUFence(m_device, fence);
}

gpu::Texture::Texture(SDL_GPUDevice* device, SDL_GPUTexture* handle, Uint32 width, Uint32 height, SDL_GPUTextureFormat format) noexcept
    : Handle(device, handle), m_width(width), m_height(height), m_format(format)
{
    // empty
}
