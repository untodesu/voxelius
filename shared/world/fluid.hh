#ifndef B347B878_D23E_45CF_A346_EDD80070F951
#define B347B878_D23E_45CF_A346_EDD80070F951

#include "core/identifier.hh"

enum fluid_gravity : unsigned {
    FLUID_GRAVITY_DOWN,
    FLUID_GRAVITY_UP,
};

using fluid_id_type = std::uint32_t;
constexpr static fluid_id_type FLUID_ID_NULL = 0;
constexpr static fluid_id_type FLUID_ID_MAX = std::numeric_limits<fluid_id_type>::max();

struct FluidDefinition final {
    FluidDefinition(void) = default;

    fluid_gravity gravity;

    bool opaque;

    std::optional<unsigned> max_level;

    float fog_density;
    Eigen::Vector3f fog_color;

    std::optional<unsigned> tint_index;

    std::vector<Identifier> still_textures;
    std::vector<Identifier> flowing_textures;
};

#endif /* B347B878_D23E_45CF_A346_EDD80070F951 */
