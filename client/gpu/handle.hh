#ifndef BB6A992F_A9DE_4F58_BC5D_867173FFACAD
#define BB6A992F_A9DE_4F58_BC5D_867173FFACAD

namespace gpu
{
template<typename T, auto ReleaseFunc>
class Handle {
public:
    Handle(void) noexcept = default;
    virtual ~Handle(void) noexcept;

    Handle(const Handle<T, ReleaseFunc>& other) = delete;
    Handle<T, ReleaseFunc>& operator=(const Handle<T, ReleaseFunc>& other) = delete;

    Handle(Handle<T, ReleaseFunc>&& other) noexcept;
    Handle<T, ReleaseFunc>& operator=(Handle<T, ReleaseFunc>&& other) noexcept;

    constexpr T* get(void) const noexcept;
    constexpr operator T*(void) const noexcept;

    constexpr bool is_valid(void) const noexcept;

    void reset(void) noexcept;

protected:
    Handle(SDL_GPUDevice* device, T* handle) noexcept;

    SDL_GPUDevice* m_device { nullptr };
    T* m_handle { nullptr };
};
} // namespace gpu

template<typename T, auto ReleaseFunc>
gpu::Handle<T, ReleaseFunc>::~Handle(void) noexcept
{
    reset();
}

template<typename T, auto ReleaseFunc>
gpu::Handle<T, ReleaseFunc>::Handle(SDL_GPUDevice* device, T* handle) noexcept : m_device(device), m_handle(handle)
{
    // empty
}

template<typename T, auto ReleaseFunc>
gpu::Handle<T, ReleaseFunc>::Handle(Handle<T, ReleaseFunc>&& other) noexcept : m_device(other.m_device), m_handle(other.m_handle)
{
    other.m_device = nullptr;
    other.m_handle = nullptr;
}

template<typename T, auto ReleaseFunc>
gpu::Handle<T, ReleaseFunc>& gpu::Handle<T, ReleaseFunc>::operator=(Handle<T, ReleaseFunc>&& other) noexcept
{
    if(this != &other) {
        reset();

        m_device = other.m_device;
        m_handle = other.m_handle;

        other.m_device = nullptr;
        other.m_handle = nullptr;
    }

    return *this;
}

template<typename T, auto ReleaseFunc>
constexpr T* gpu::Handle<T, ReleaseFunc>::get(void) const noexcept
{
    return m_handle;
}

template<typename T, auto ReleaseFunc>
constexpr gpu::Handle<T, ReleaseFunc>::operator T*(void) const noexcept
{
    return m_handle;
}

template<typename T, auto ReleaseFunc>
constexpr bool gpu::Handle<T, ReleaseFunc>::is_valid(void) const noexcept
{
    return m_handle != nullptr;
}

template<typename T, auto ReleaseFunc>
void gpu::Handle<T, ReleaseFunc>::reset(void) noexcept
{
    if(m_handle) {
        ReleaseFunc(m_device, m_handle);
        m_handle = nullptr;
    }
}

#endif /* BB6A992F_A9DE_4F58_BC5D_867173FFACAD */
