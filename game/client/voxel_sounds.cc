#include "client/pch.hh"

#include "client/voxel_sounds.hh"

#include "client/sound_effect.hh"

constexpr static std::size_t NUM_SURFACES = static_cast<std::size_t>(voxel_surface::COUNT);

static std::vector<resource_ptr<SoundEffect>> footsteps_sounds[NUM_SURFACES];
static std::mt19937_64 randomizer;

static void add_footsteps_effect(voxel_surface surface, const char* name)
{
    if(auto effect = resource::load<SoundEffect>(name)) {
        auto surface_index = static_cast<std::size_t>(surface);
        footsteps_sounds[surface_index].push_back(effect);
    }
}

static resource_ptr<SoundEffect> get_footsteps_effect(voxel_surface surface)
{
    auto surface_index = static_cast<std::size_t>(surface);

    if(surface_index >= NUM_SURFACES) {
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

void voxel_sounds::init(void)
{
    add_footsteps_effect(voxel_surface::DEFAULT, "sounds/surface/default1.wav");
    add_footsteps_effect(voxel_surface::DEFAULT, "sounds/surface/default2.wav");
    add_footsteps_effect(voxel_surface::DEFAULT, "sounds/surface/default3.wav");
    add_footsteps_effect(voxel_surface::DEFAULT, "sounds/surface/default4.wav");

    add_footsteps_effect(voxel_surface::DIRT, "sounds/surface/dirt1.wav");

    add_footsteps_effect(voxel_surface::GRASS, "sounds/surface/grass1.wav");
    add_footsteps_effect(voxel_surface::GRASS, "sounds/surface/grass2.wav");
    add_footsteps_effect(voxel_surface::GRASS, "sounds/surface/grass3.wav");

    add_footsteps_effect(voxel_surface::GRAVEL, "sounds/surface/gravel1.wav");

    add_footsteps_effect(voxel_surface::SAND, "sounds/surface/sand1.wav");
    add_footsteps_effect(voxel_surface::SAND, "sounds/surface/sand2.wav");

    add_footsteps_effect(voxel_surface::WOOD, "sounds/surface/wood1.wav");
    add_footsteps_effect(voxel_surface::WOOD, "sounds/surface/wood2.wav");
    add_footsteps_effect(voxel_surface::WOOD, "sounds/surface/wood3.wav");
}

void voxel_sounds::deinit(void)
{
    for(std::size_t i = 0; i < NUM_SURFACES; ++i) {
        footsteps_sounds[i].clear();
    }
}

resource_ptr<SoundEffect> voxel_sounds::get_footsteps(voxel_surface surface)
{
    if(auto effect = get_footsteps_effect(surface)) {
        return effect;
    }

    if(auto effect = get_footsteps_effect(voxel_surface::DEFAULT)) {
        return effect;
    }

    return nullptr;
}

resource_ptr<SoundEffect> voxel_sounds::get_placebreak(voxel_surface surface)
{
    return nullptr;
}
