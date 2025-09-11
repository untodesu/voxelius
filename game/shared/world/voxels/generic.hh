#pragma once

#include "shared/world/voxel.hh"

namespace world::voxels
{
class GenericCube final : public Voxel {
public:
    template<typename... TexturesT>
    requires(std::is_convertible_v<TexturesT, std::string_view> && ...)
    explicit GenericCube(std::string_view name, VoxelRender render_mode, bool animated, VoxelMaterial surface_material, VoxelTouch touch,
        const glm::fvec3& touch_values, TexturesT&&... textures) noexcept
    {
        set_name(name);

        set_shape(VoxelShape::CUBE);
        set_render_mode(render_mode);
        set_animated(animated);

        set_surface_material(surface_material);
        set_touch_values(touch_values);
        set_touch_type(touch);

        add_texture_default(std::forward<TexturesT>(textures)...);
    }
};
} // namespace world::voxels
