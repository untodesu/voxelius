#ifndef DD203EC0_CAAF_417C_A6B7_8A7969AD56E9
#define DD203EC0_CAAF_417C_A6B7_8A7969AD56E9

#include "client/gpu/handle.hh"

namespace gpu
{
class Shader final : public Handle<SDL_GPUShader, SDL_ReleaseGPUShader> {
public:
    Shader(void) noexcept = default;

    static Shader create(const SDL_GPUShaderCreateInfo& info) noexcept;

private:
    using Handle::Handle;
};
} // namespace gpu

#endif /* DD203EC0_CAAF_417C_A6B7_8A7969AD56E9 */
