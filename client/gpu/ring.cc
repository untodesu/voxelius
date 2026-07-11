#include "client/pch.hh"

#include "client/gpu/ring.hh"

gpu::Ring gpu::Ring::create(std::size_t capacity, SDL_GPUBufferUsageFlags usage, std::size_t backing)
{
    assert(backing > 0);

    Ring ring;
    ring.m_buffers.reserve(backing);

    for(std::size_t i = 0; i < backing; ++i) {
        ring.m_buffers.push_back(Buffer::create(capacity, usage));
    }

    return ring;
}

SDL_GPUBuffer* gpu::Ring::write_streamed(SDL_GPUCopyPass* copy_pass, std::span<const std::byte> data)
{
    auto& buffer = m_buffers[m_current_frame];
    buffer.write_streamed(copy_pass, data);
    return buffer.get();
}

void gpu::Ring::update_late(void)
{
    m_current_frame += 1;
    m_current_frame %= m_buffers.size();
}
