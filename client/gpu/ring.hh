#ifndef A7F5D6CE_3F6A_4D39_8644_580D9FCED71C
#define A7F5D6CE_3F6A_4D39_8644_580D9FCED71C

#include "client/gpu/buffer.hh"

namespace gpu
{
class Ring final {
public:
    Ring(void) noexcept = default;

    static Ring create(std::size_t capacity, SDL_GPUBufferUsageFlags usage, std::size_t backing = 2) noexcept;

    SDL_GPUBuffer* write_streamed(SDL_GPUCopyPass* copy_pass, std::span<const std::byte> data);

    void update_late(void) noexcept;

    constexpr std::size_t backing(void) const noexcept;

private:
    std::vector<Buffer> m_buffers;
    std::size_t m_current_frame { 0 };
};
} // namespace gpu

constexpr std::size_t gpu::Ring::backing(void) const noexcept
{
    return m_buffers.size();
}

#endif /* A7F5D6CE_3F6A_4D39_8644_580D9FCED71C */
