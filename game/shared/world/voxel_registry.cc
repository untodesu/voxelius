#include "shared/pch.hh"

#include "shared/world/voxel_registry.hh"

static std::uint64_t registry_checksum = 0U;
emhash8::HashMap<std::string, voxel_id> world::voxel_registry::names;
std::vector<std::unique_ptr<world::Voxel>> world::voxel_registry::voxels;

static void recalculate_checksum(void)
{
    registry_checksum = 0U;

    for(const auto& voxel : world::voxel_registry::voxels) {
        registry_checksum = voxel->get_checksum(registry_checksum);
    }
}

world::Voxel* world::voxel_registry::register_voxel(const VoxelBuilder& builder)
{
    assert(builder.get_name().size());
    assert(nullptr == find(builder.get_name()));

    const auto id = static_cast<voxel_id>(1 + voxels.size());

    std::unique_ptr<Voxel> voxel(builder.build(id));
    names.emplace(std::string(builder.get_name()), id);
    voxels.push_back(std::move(voxel));

    recalculate_checksum();

    return voxels.back().get();
}

world::Voxel* world::voxel_registry::find(std::string_view name)
{
    const auto it = names.find(std::string(name));

    if(it == names.end()) {
        return nullptr;
    }

    return voxels[it->second - 1].get();
}

world::Voxel* world::voxel_registry::find(voxel_id id)
{
    if(id == NULL_VOXEL_ID || id > voxels.size()) {
        return nullptr;
    }

    return voxels[id - 1].get();
}

void world::voxel_registry::purge(void)
{
    registry_checksum = 0U;
    voxels.clear();
    names.clear();
}

std::uint64_t world::voxel_registry::get_checksum(void)
{
    return registry_checksum;
}
