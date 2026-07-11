#ifndef BB6A992F_A9DE_4F58_BC5D_867173FFACAD
#define BB6A992F_A9DE_4F58_BC5D_867173FFACAD

namespace gpu
{
template<typename T, auto ReleaseFunc>
class Handle {
public:
    Handle(void) = default;
    virtual ~Handle(void);

    Handle(const Handle<T, ReleaseFunc>& other) = delete;
    Handle<T, ReleaseFunc>& operator=(const Handle<T, ReleaseFunc>& other) = delete;

    Handle(Handle<T, ReleaseFunc>&& other);
    Handle<T, ReleaseFunc>& operator=(Handle<T, ReleaseFunc>&& other);

    constexpr T* get(void) const;
    constexpr operator T*(void) const;

    constexpr bool is_valid(void) const;

    void reset(void);

protected:
    Handle(SDL_GPUDevice* device, T* handle);

    SDL_GPUDevice* m_device { nullptr };
    T* m_handle { nullptr };
};
} // namespace gpu

template<typename T, auto ReleaseFunc>
gpu::Handle<T, ReleaseFunc>::~Handle(void)
{
    reset();
}

template<typename T, auto ReleaseFunc>
gpu::Handle<T, ReleaseFunc>::Handle(SDL_GPUDevice* device, T* handle) : m_device(device), m_handle(handle)
{
    // empty
}

template<typename T, auto ReleaseFunc>
gpu::Handle<T, ReleaseFunc>::Handle(Handle<T, ReleaseFunc>&& other) : m_device(other.m_device), m_handle(other.m_handle)
{
    other.m_device = nullptr;
    other.m_handle = nullptr;
}

template<typename T, auto ReleaseFunc>
gpu::Handle<T, ReleaseFunc>& gpu::Handle<T, ReleaseFunc>::operator=(Handle<T, ReleaseFunc>&& other)
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
constexpr T* gpu::Handle<T, ReleaseFunc>::get(void) const
{
    return m_handle;
}

template<typename T, auto ReleaseFunc>
constexpr gpu::Handle<T, ReleaseFunc>::operator T*(void) const
{
    return m_handle;
}

template<typename T, auto ReleaseFunc>
constexpr bool gpu::Handle<T, ReleaseFunc>::is_valid(void) const
{
    return static_cast<bool>(m_handle);
}

template<typename T, auto ReleaseFunc>
void gpu::Handle<T, ReleaseFunc>::reset(void)
{
    if(m_handle) {
        ReleaseFunc(m_device, m_handle);
        m_handle = nullptr;
    }
}

#endif /* BB6A992F_A9DE_4F58_BC5D_867173FFACAD */
