#include "shared/pch.hh"

#include "shared/world/voxel_registry.hh"

static std::uint64_t registry_checksum = 0U;
emhash8::HashMap<std::string, voxel_id> world::voxel_registry::names;
std::vector<std::shared_ptr<world::Voxel>> world::voxel_registry::voxels;

static void recalculate_checksum(void)
{
    registry_checksum = 0U;

    for(const auto& voxel : world::voxel_registry::voxels) {
        registry_checksum = voxel->calculate_checksum(registry_checksum);
    }
}

world::Voxel* world::voxel_registry::register_voxel(const Voxel& voxel_template)
{
    assert(voxel_template.get_name().size());
    assert(nullptr == find(voxel_template.get_name()));

    const auto id = static_cast<voxel_id>(voxels.size());

    auto voxel = voxel_template.clone();
    voxel->set_id(id);

    names.emplace(std::string(voxel_template.get_name()), id);
    voxels.push_back(std::move(voxel));

    recalculate_checksum();

    return voxels.back().get();
}

world::Voxel* world::voxel_registry::find(std::string_view name)
{
    const auto it = names.find(name);

    if(it == names.end()) {
        return nullptr;
    }

    return voxels[it->second].get();
}

world::Voxel* world::voxel_registry::find(voxel_id id)
{
    if(id >= voxels.size()) {
        return nullptr;
    }

    return voxels[id].get();
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
