#ifndef A9C3EC63_132F_433D_8242_CE4255A727C9
#define A9C3EC63_132F_433D_8242_CE4255A727C9

#include "client/gpu/handle.hh"

namespace gpu
{
class Sampler final : public Handle<SDL_GPUSampler, SDL_ReleaseGPUSampler> {
public:
    Sampler(void) noexcept = default;

    static Sampler create(const SDL_GPUSamplerCreateInfo& info) noexcept;
    static Sampler create(SDL_GPUFilter filter, SDL_GPUSamplerMipmapMode mipmap_mode, SDL_GPUSamplerAddressMode address_mode) noexcept;

private:
    using Handle::Handle;
};
} // namespace gpu

#endif /* A9C3EC63_132F_433D_8242_CE4255A727C9 */
