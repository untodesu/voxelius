#ifndef BAF67FCA_C1FC_4DEC_88DC_E0C12DDE4C47
#define BAF67FCA_C1FC_4DEC_88DC_E0C12DDE4C47

#include "client/gpu/handle.hh"

namespace gpu
{
class Buffer final : public Handle<SDL_GPUBuffer, SDL_ReleaseGPUBuffer> {
public:
    Buffer(void) = default;

    static Buffer create(const SDL_GPUBufferCreateInfo& info);
    static Buffer create(std::size_t capacity, SDL_GPUBufferUsageFlags usage);

    void write_streamed(SDL_GPUCopyPass* copy_pass, std::span<const std::byte> data, std::ptrdiff_t offset = 0);
    void write_blocking(std::span<const std::byte> data, std::ptrdiff_t offset = 0);

    constexpr std::size_t capacity(void) const;
    constexpr std::size_t size(void) const;

private:
    explicit Buffer(SDL_GPUDevice* device, SDL_GPUBuffer* handle, std::size_t capacity, SDL_GPUBufferUsageFlags usage);

    void ensure_capacity(std::size_t capacity);

    SDL_GPUBufferUsageFlags m_usage { 0 };
    std::size_t m_capacity { 0 };
    std::size_t m_size { 0 };
};
} // namespace gpu

constexpr std::size_t gpu::Buffer::capacity(void) const
{
    return m_capacity;
}

constexpr std::size_t gpu::Buffer::size(void) const
{
    return m_size;
}

#endif /* BAF67FCA_C1FC_4DEC_88DC_E0C12DDE4C47 */
