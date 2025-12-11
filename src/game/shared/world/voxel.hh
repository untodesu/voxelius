#pragma once

#include "core/math/aabb.hh"

#include "shared/types.hh"

namespace world
{
class Dimension;
} // namespace world

namespace world
{
enum VoxelRender : unsigned int {
    VRENDER_NONE = 0U, ///< The voxel is not rendered at all
    VRENDER_OPAQUE,    ///< The voxel is fully opaque
    VRENDER_BLEND,     ///< The voxel is blended (e.g. water, glass)
};

enum VoxelShape : unsigned int {
    VSHAPE_CUBE = 0U, ///< Full cube shape
    VSHAPE_CROSS,     ///< TODO: Cross shape
    VSHAPE_MODEL,     ///< TODO: Custom model shape
};

enum VoxelFace : unsigned int {
    VFACE_NORTH = 0U, ///< Positive Z face
    VFACE_SOUTH,      ///< Negative Z face
    VFACE_EAST,       ///< Positive X face
    VFACE_WEST,       ///< Negative X face
    VFACE_TOP,        ///< Positive Y face
    VFACE_BOTTOM,     ///< Negative Y face
    VFACE_CROSS_NWSE, ///< Diagonal cross face northwest-southeast
    VFACE_CROSS_NESW, ///< Diagonal cross face northeast-southwest
    VFACE_COUNT
};

enum VoxelTouch : unsigned int {
    VTOUCH_NONE = 0xFFFFU,
    VTOUCH_SOLID = 0U, ///< The entity is stopped in its tracks
    VTOUCH_BOUNCE,     ///< The entity bounces back with some energy loss
    VTOUCH_SINK,       ///< The entity phases/sinks through the voxel
};

enum VoxelMaterial : unsigned int {
    VMAT_UNKNOWN = 0xFFFFU,
    VMAT_DEFAULT = 0U,
    VMAT_STONE,
    VMAT_DIRT,
    VMAT_GLASS,
    VMAT_GRASS,
    VMAT_GRAVEL,
    VMAT_METAL,
    VMAT_SAND,
    VMAT_WOOD,
    VMAT_SLOSH,
    VMAT_COUNT
};

enum VoxelVisBits : unsigned int {
    VVIS_NORTH = 1U << VFACE_NORTH, ///< Positive Z
    VVIS_SOUTH = 1U << VFACE_SOUTH, ///< Negative Z
    VVIS_EAST = 1U << VFACE_EAST,   ///< Positive X
    VVIS_WEST = 1U << VFACE_WEST,   ///< Negative X
    VVIS_UP = 1U << VFACE_TOP,      ///< Positive Y
    VVIS_DOWN = 1U << VFACE_BOTTOM, ///< Negative Y
};
} // namespace world

namespace world
{
using VoxelOnPlaceFunc = std::function<void(Dimension*, const voxel_pos&)>;
using VoxelOnRemoveFunc = std::function<void(Dimension*, const voxel_pos&)>;
using VoxelOnTickFunc = std::function<void(Dimension*, const voxel_pos&)>;
} // namespace world

namespace world
{
class Voxel {
public:
    Voxel(void) = default;
    explicit Voxel(const Voxel& source, voxel_id id) noexcept;

    void on_place(Dimension* dimension, const voxel_pos& vpos) const;
    void on_remove(Dimension* dimension, const voxel_pos& vpos) const;
    void on_tick(Dimension* dimension, const voxel_pos& vpos) const;

    constexpr std::string_view get_name(void) const noexcept;
    constexpr voxel_id get_id(void) const noexcept;

    constexpr VoxelRender get_render_mode(void) const noexcept;
    constexpr VoxelShape get_shape(void) const noexcept;
    constexpr bool is_animated(void) const noexcept;

    constexpr VoxelTouch get_touch_type(void) const noexcept;
    constexpr const glm::fvec3& get_touch_values(void) const noexcept;
    constexpr VoxelMaterial get_surface_material(void) const noexcept;

    constexpr const math::AABBf& get_collision(void) const noexcept;

    constexpr const std::vector<std::string>& get_default_textures(void) const noexcept;
    constexpr const std::vector<std::string>& get_face_textures(VoxelFace face) const noexcept;
    constexpr std::size_t get_cached_face_offset(VoxelFace face) const noexcept;
    constexpr std::size_t get_cached_face_plane(VoxelFace face) const noexcept;

    template<VoxelRender RenderMode>
    constexpr bool is_render_mode(void) const noexcept;
    template<VoxelShape Shape>
    constexpr bool is_shape(void) const noexcept;
    template<VoxelTouch TouchType>
    constexpr bool is_touch_type(void) const noexcept;
    template<VoxelMaterial Material>
    constexpr bool is_surface_material(void) const noexcept;

    /// Non-model voxel shapes support texture variation based on the
    /// voxel position on the world; this method handles the math behind this
    /// @param face The face of the voxel to get the texture index for
    /// @param vpos The absolute voxel position to get the texture index for
    /// @return The index of the texture to use for the given face at the given position
    /// @remarks On client-side: plane[get_cached_face_plane][get_cached_face_offset + thisFunctionResult]
    std::size_t get_random_texture_index(VoxelFace face, const voxel_pos& vpos) const;

    /// Assign cached plane index and plane offset for a given face
    /// @param face The face to assign the cache for
    /// @param offset The offset to assign to the face
    /// @param plane The plane index to assign to the face
    void set_face_cache(VoxelFace face, std::size_t offset, std::size_t plane);

    /// Calculate a checksum for the voxel's properties
    /// @param combine An optional initial checksum to combine with
    /// @return The calculated checksum
    std::uint64_t get_checksum(std::uint64_t combine = 0U) const;

protected:
    std::string m_name;
    voxel_id m_id { NULL_VOXEL_ID };

    VoxelRender m_render_mode { VRENDER_OPAQUE };
    VoxelShape m_shape { VSHAPE_CUBE };
    bool m_animated { false };

    VoxelTouch m_touch_type { VTOUCH_SOLID };
    glm::fvec3 m_touch_values { 0.0f };
    VoxelMaterial m_surface_material { VMAT_DEFAULT };

    math::AABBf m_collision { { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f } };

    std::vector<std::string> m_default_textures;
    std::array<std::vector<std::string>, VFACE_COUNT> m_face_textures;
    std::array<std::size_t, VFACE_COUNT> m_cached_face_offsets;
    std::array<std::size_t, VFACE_COUNT> m_cached_face_planes;

    VoxelOnPlaceFunc m_on_place;
    VoxelOnRemoveFunc m_on_remove;
    VoxelOnTickFunc m_on_tick;
};
} // namespace world

namespace world
{
class VoxelBuilder final : public Voxel {
public:
    VoxelBuilder(void) = default;
    explicit VoxelBuilder(std::string_view name);

    void set_on_place(VoxelOnPlaceFunc func) noexcept;
    void set_on_remove(VoxelOnRemoveFunc func) noexcept;
    void set_on_tick(VoxelOnTickFunc func) noexcept;

    void set_name(std::string_view name) noexcept;

    void set_render_mode(VoxelRender mode) noexcept;
    void set_shape(VoxelShape shape) noexcept;
    void set_animated(bool animated) noexcept;

    void set_touch_type(VoxelTouch type) noexcept;
    void set_touch_values(const glm::fvec3& values) noexcept;
    void set_surface_material(VoxelMaterial material) noexcept;

    void set_collision(const math::AABBf& box) noexcept;

    void add_default_texture(std::string_view path);
    void add_face_texture(VoxelFace face, std::string_view path);

    std::unique_ptr<Voxel> build(voxel_id id) const;
};
} // namespace world

constexpr std::string_view world::Voxel::get_name(void) const noexcept
{
    return m_name;
}

constexpr voxel_id world::Voxel::get_id(void) const noexcept
{
    return m_id;
}

constexpr world::VoxelRender world::Voxel::get_render_mode(void) const noexcept
{
    return m_render_mode;
}

constexpr world::VoxelShape world::Voxel::get_shape(void) const noexcept
{
    return m_shape;
}

constexpr bool world::Voxel::is_animated(void) const noexcept
{
    return m_animated;
}

constexpr world::VoxelTouch world::Voxel::get_touch_type(void) const noexcept
{
    return m_touch_type;
}

constexpr const glm::fvec3& world::Voxel::get_touch_values(void) const noexcept
{
    return m_touch_values;
}

constexpr world::VoxelMaterial world::Voxel::get_surface_material(void) const noexcept
{
    return m_surface_material;
}

constexpr const math::AABBf& world::Voxel::get_collision(void) const noexcept
{
    return m_collision;
}

constexpr const std::vector<std::string>& world::Voxel::get_default_textures(void) const noexcept
{
    return m_default_textures;
}

constexpr const std::vector<std::string>& world::Voxel::get_face_textures(VoxelFace face) const noexcept
{
    assert(face <= m_face_textures.size());

    if(m_face_textures[face].empty()) {
        return m_default_textures;
    }

    return m_face_textures[face];
}

constexpr std::size_t world::Voxel::get_cached_face_offset(VoxelFace face) const noexcept
{
    assert(face <= m_cached_face_offsets.size());

    return m_cached_face_offsets[face];
}

constexpr std::size_t world::Voxel::get_cached_face_plane(VoxelFace face) const noexcept
{
    assert(face <= m_cached_face_planes.size());

    return m_cached_face_planes[face];
}

template<world::VoxelRender RenderMode>
constexpr bool world::Voxel::is_render_mode(void) const noexcept
{
    return m_render_mode == RenderMode;
}

template<world::VoxelShape Shape>
constexpr bool world::Voxel::is_shape(void) const noexcept
{
    return m_shape == Shape;
}

template<world::VoxelTouch TouchType>
constexpr bool world::Voxel::is_touch_type(void) const noexcept
{
    return m_touch_type == TouchType;
}

template<world::VoxelMaterial Material>
constexpr bool world::Voxel::is_surface_material(void) const noexcept
{
    return m_surface_material == Material;
}
