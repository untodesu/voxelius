#include "client/pch.hh"

#include "client/world/voxel_sounds.hh"

#include "client/resource/sound_effect.hh"

static std::vector<resource_ptr<SoundEffect>> footsteps_sounds[world::VMAT_COUNT];
static std::mt19937_64 randomizer;

static void add_footsteps_effect(world::VoxelMaterial material, std::string_view name)
{
    if(auto effect = resource::load<SoundEffect>(name)) {
        footsteps_sounds[material].push_back(effect);
    }
}

static resource_ptr<SoundEffect> get_footsteps_effect(world::VoxelMaterial material)
{
    auto surface_index = static_cast<std::size_t>(material);

    if(surface_index >= world::VMAT_COUNT) {
        // Surface index out of range
        return nullptr;
    }

    const auto& sounds = footsteps_sounds[surface_index];

    if(sounds.empty()) {
        // No sounds for this surface
        return nullptr;
    }

    auto dist = std::uniform_int_distribution<std::size_t>(0, sounds.size() - 1);
    return sounds.at(dist(randomizer));
}

void world::voxel_sounds::init(void)
{
    add_footsteps_effect(VMAT_DEFAULT, "sounds/surface/default1.wav");
    add_footsteps_effect(VMAT_DEFAULT, "sounds/surface/default2.wav");
    add_footsteps_effect(VMAT_DEFAULT, "sounds/surface/default3.wav");
    add_footsteps_effect(VMAT_DEFAULT, "sounds/surface/default4.wav");

    add_footsteps_effect(VMAT_DIRT, "sounds/surface/dirt1.wav");

    add_footsteps_effect(VMAT_GRASS, "sounds/surface/grass1.wav");
    add_footsteps_effect(VMAT_GRASS, "sounds/surface/grass2.wav");
    add_footsteps_effect(VMAT_GRASS, "sounds/surface/grass3.wav");

    add_footsteps_effect(VMAT_GRAVEL, "sounds/surface/gravel1.wav");

    add_footsteps_effect(VMAT_SAND, "sounds/surface/sand1.wav");
    add_footsteps_effect(VMAT_SAND, "sounds/surface/sand2.wav");

    add_footsteps_effect(VMAT_WOOD, "sounds/surface/wood1.wav");
    add_footsteps_effect(VMAT_WOOD, "sounds/surface/wood2.wav");
    add_footsteps_effect(VMAT_WOOD, "sounds/surface/wood3.wav");
}

void world::voxel_sounds::shutdown(void)
{
    for(std::size_t i = 0; i < world::VMAT_COUNT; ++i) {
        footsteps_sounds[i].clear();
    }
}

resource_ptr<SoundEffect> world::voxel_sounds::get_footsteps(world::VoxelMaterial material)
{
    if(auto effect = get_footsteps_effect(material)) {
        return effect;
    }

    if(auto effect = get_footsteps_effect(VMAT_DEFAULT)) {
        return effect;
    }

    return nullptr;
}

resource_ptr<SoundEffect> world::voxel_sounds::get_placebreak(world::VoxelMaterial material)
{
    return nullptr;
}
