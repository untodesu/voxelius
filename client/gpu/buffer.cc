#include "client/pch.hh"

#include "client/gpu/buffer.hh"

#include "core/exception.hh"

#include "client/globals.hh"

gpu::Buffer gpu::Buffer::create(const SDL_GPUBufferCreateInfo& info) noexcept
{
    auto handle = SDL_CreateGPUBuffer(globals::gpu_device, &info);

    if(handle == nullptr) {
        LOG_WARNING("SDL_CreateGPUBuffer failed: {}", SDL_GetError());
        return {};
    }

    return Buffer(globals::gpu_device, handle, info.size, info.usage);
}

gpu::Buffer gpu::Buffer::create(std::size_t capacity, SDL_GPUBufferUsageFlags usage) noexcept
{
    SDL_GPUBufferCreateInfo info {};
    info.size = static_cast<Uint32>(capacity);
    info.usage = usage;

    return create(info);
}

void gpu::Buffer::write_streamed(SDL_GPUCopyPass* copy_pass, std::span<const std::byte> data, std::ptrdiff_t offset)
{
    assert(offset >= 0);

    ensure_capacity(static_cast<std::size_t>(offset) + data.size());

    SDL_GPUTransferBufferCreateInfo transfer_info {};
    transfer_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    transfer_info.size = static_cast<Uint32>(data.size());

    auto tbuf = SDL_CreateGPUTransferBuffer(m_device, &transfer_info);
    vx::throw_if_fmt(tbuf == nullptr, "SDL_CreateGPUTransferBuffer failed: {}", SDL_GetError());

    auto mapped = SDL_MapGPUTransferBuffer(m_device, tbuf, true);
    vx::throw_if_fmt(mapped == nullptr, "SDL_MapGPUTransferBuffer failed: {}", SDL_GetError());

    std::memcpy(mapped, data.data(), data.size());

    SDL_UnmapGPUTransferBuffer(m_device, tbuf);

    SDL_GPUTransferBufferLocation source {};
    source.transfer_buffer = tbuf;
    source.offset = 0;

    SDL_GPUBufferRegion destination {};
    destination.buffer = m_handle;
    destination.offset = static_cast<Uint32>(offset);
    destination.size = static_cast<Uint32>(data.size());

    SDL_UploadToGPUBuffer(copy_pass, &source, &destination, false);

    SDL_ReleaseGPUTransferBuffer(m_device, tbuf);

    m_size = std::max(m_size, static_cast<std::size_t>(offset) + data.size());
}

void gpu::Buffer::write_blocking(std::span<const std::byte> data, std::ptrdiff_t offset)
{
    auto cmds = SDL_AcquireGPUCommandBuffer(m_device);
    vx::throw_if_fmt(cmds == nullptr, "SDL_AcquireGPUCommandBuffer failed: {}", SDL_GetError());

    auto pass = SDL_BeginGPUCopyPass(cmds);
    vx::throw_if(pass == nullptr, "SDL_BeginGPUCopyPass returned null!");

    write_streamed(pass, data, offset);

    SDL_EndGPUCopyPass(pass);

    auto fence = SDL_SubmitGPUCommandBufferAndAcquireFence(cmds);
    SDL_WaitForGPUFences(m_device, true, &fence, 1);
    SDL_ReleaseGPUFence(m_device, fence);
}

void gpu::Buffer::ensure_capacity(std::size_t required)
{
    if(required > m_capacity) {
        auto new_capacity = std::max(required, m_capacity * 2);

        SDL_GPUBufferCreateInfo buffer_info {};
        buffer_info.size = static_cast<Uint32>(new_capacity);
        buffer_info.usage = m_usage;

        auto new_handle = SDL_CreateGPUBuffer(m_device, &buffer_info);
        vx::throw_if_fmt(new_handle == nullptr, "SDL_CreateGPUBuffer failed: {}", SDL_GetError());

        // NOTE: SDL seems to defer handle release until the GPU
        // is truly done with the object, so it's probably safe
        // to even release the old handle mid-frame
        reset();

        m_handle = new_handle;
        m_capacity = new_capacity;
    }
}

gpu::Buffer::Buffer(SDL_GPUDevice* device, SDL_GPUBuffer* handle, std::size_t capacity, SDL_GPUBufferUsageFlags usage) noexcept
    : Handle(device, handle), m_usage(usage), m_capacity(capacity), m_size(0)
{
    // empty
}
