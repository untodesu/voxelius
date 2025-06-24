#include "shared/pch.hh"

#include "shared/voxel_registry.hh"

#include "core/crc64.hh"

std::unordered_map<std::string, VoxelInfoBuilder> voxel_registry::builders = {};
std::unordered_map<std::string, voxel_id> voxel_registry::names = {};
std::vector<std::shared_ptr<VoxelInfo>> voxel_registry::voxels = {};

VoxelInfoBuilder::VoxelInfoBuilder(const char* name, voxel_type type, bool animated, bool blending)
{
    prototype.name = name;
    prototype.type = type;
    prototype.animated = animated;
    prototype.blending = blending;

    switch(type) {
        case voxel_type::CUBE:
            prototype.textures.resize(static_cast<std::size_t>(voxel_face::CUBE__NR));
            break;
        case voxel_type::CROSS:
            prototype.textures.resize(static_cast<std::size_t>(voxel_face::CROSS__NR));
            break;
        case voxel_type::MODEL:
            // Custom models should use a different texture
            // resource management that is not a voxel atlas
            // TODO: actually implement custom models lol
            prototype.textures.resize(0);
            break;
        default:
            // Something really bad should happen if we end up here.
            // The outside code would static_cast an int to VoxelType
            // and possibly fuck a lot of things up to cause this
            spdlog::critical("voxel_registry: {}: unknown voxel type {}", name, static_cast<int>(type));
            std::terminate();
    }

    // Physics properties
    prototype.touch_type = voxel_touch::SOLID;
    prototype.touch_values = glm::fvec3(0.0f, 0.0f, 0.0f);
    prototype.surface = voxel_surface::DEFAULT;

    // Things set in future by item_def
    prototype.item_pick = NULL_ITEM_ID;
}

VoxelInfoBuilder& VoxelInfoBuilder::add_texture_default(const char* texture)
{
    default_texture.paths.push_back(texture);
    return *this;
}

VoxelInfoBuilder& VoxelInfoBuilder::add_texture(voxel_face face, const char* texture)
{
    const auto index = static_cast<std::size_t>(face);
    prototype.textures[index].paths.push_back(texture);
    return *this;
}

VoxelInfoBuilder& VoxelInfoBuilder::set_touch(voxel_touch type, const glm::fvec3& values)
{
    prototype.touch_type = type;
    prototype.touch_values = values;
    return *this;
}

VoxelInfoBuilder& VoxelInfoBuilder::set_surface(voxel_surface surface)
{
    prototype.surface = surface;
    return *this;
}

voxel_id VoxelInfoBuilder::build(void) const
{
    const auto it = voxel_registry::names.find(prototype.name);

    if(it != voxel_registry::names.cend()) {
        spdlog::warn("voxel_registry: cannot build {}: name already present", prototype.name);
        return it->second;
    }

    std::size_t state_count;

    switch(prototype.type) {
        case voxel_type::CUBE:
        case voxel_type::CROSS:
        case voxel_type::MODEL:
            state_count = 1;
            break;
        default:
            // Something really bad should happen if we end up here.
            // The outside code would static_cast an int to VoxelType
            // and possibly fuck a lot of things up to cause this
            spdlog::critical("voxel_registry: {}: unknown voxel type {}", prototype.name, static_cast<int>(prototype.type));
            std::terminate();
    }

    if((voxel_registry::voxels.size() + state_count) >= MAX_VOXEL_ID) {
        spdlog::critical("voxel_registry: voxel registry overflow");
        std::terminate();
    }

    auto new_info = std::make_shared<VoxelInfo>();
    new_info->name = prototype.name;
    new_info->type = prototype.type;
    new_info->animated = prototype.animated;
    new_info->blending = prototype.blending;

    new_info->textures.resize(prototype.textures.size());

    for(std::size_t i = 0; i < prototype.textures.size(); ++i) {
        if(prototype.textures[i].paths.empty()) {
            new_info->textures[i].paths = default_texture.paths;
            new_info->textures[i].cached_offset = SIZE_MAX;
            new_info->textures[i].cached_plane = SIZE_MAX;
        } else {
            new_info->textures[i].paths = prototype.textures[i].paths;
            new_info->textures[i].cached_offset = SIZE_MAX;
            new_info->textures[i].cached_plane = SIZE_MAX;
        }
    }

    // Physics properties
    new_info->touch_type = prototype.touch_type;
    new_info->touch_values = prototype.touch_values;
    new_info->surface = prototype.surface;

    // Things set in future by item_def
    new_info->item_pick = prototype.item_pick;

    // Base voxel identifier offset
    new_info->base_voxel = voxel_registry::voxels.size() + 1;

    for(std::size_t i = 0; i < state_count; ++i)
        voxel_registry::voxels.push_back(new_info);
    voxel_registry::names.insert_or_assign(new_info->name, new_info->base_voxel);

    return new_info->base_voxel;
}

VoxelInfoBuilder& voxel_registry::construct(const char* name, voxel_type type, bool animated, bool blending)
{
    const auto it = voxel_registry::builders.find(name);

    if(it != voxel_registry::builders.cend()) {
        return it->second;
    } else {
        return voxel_registry::builders.emplace(name, VoxelInfoBuilder(name, type, animated, blending)).first->second;
    }
}

VoxelInfo* voxel_registry::find(const char* name)
{
    const auto it = voxel_registry::names.find(name);

    if(it != voxel_registry::names.cend()) {
        return voxel_registry::find(it->second);
    } else {
        return nullptr;
    }
}

VoxelInfo* voxel_registry::find(const voxel_id voxel)
{
    if((voxel != NULL_VOXEL_ID) && (voxel <= voxel_registry::voxels.size())) {
        return voxel_registry::voxels[voxel - 1].get();
    } else {
        return nullptr;
    }
}

void voxel_registry::purge(void)
{
    voxel_registry::builders.clear();
    voxel_registry::names.clear();
    voxel_registry::voxels.clear();
}

std::uint64_t voxel_registry::calcualte_checksum(void)
{
    std::uint64_t result = 0;

    for(const std::shared_ptr<VoxelInfo>& info : voxel_registry::voxels) {
        result = crc64::get(info->name, result);
        result += static_cast<std::uint64_t>(info->type);
        result += static_cast<std::uint64_t>(info->base_voxel);
        result += info->blending ? 256 : 1;
    }

    return result;
}
