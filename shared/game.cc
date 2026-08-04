#include "shared/pch.hh"

#include "shared/game.hh"

#include "core/res/image.hh"

#include "shared/component/head.hh"
#include "shared/component/stasis.hh"
#include "shared/component/transform.hh"
#include "shared/component/velocity.hh"
#include "shared/globals.hh"
#include "shared/mod_loader.hh"
#include "shared/res/block_collision.hh"
#include "shared/res/block_model.hh"
#include "shared/res/feature.hh"
#include "shared/world/biome_map.hh"
#include "shared/world/block_collisions.hh"
#include "shared/world/world.hh"

void shared_game::init(void)
{
    Image::register_resource();

    BlockCollision::register_resource();
    BlockModel::register_resource();
    Feature::register_resource();

    Transform::register_component();
    Head_Component::register_component();
    Velocity_Component::register_component();

    biome_map::init();
    mod_loader::init();
}

void shared_game::init_late(void)
{
    block_collisions::init_late();
}

void shared_game::shutdown(void)
{
    world::purge();

    block_collisions::shutdown();

    mod_loader::shutdown();
}

void shared_game::fixed_update(void)
{
    ZoneScoped;

    // NOTE: this freezes entities that are out
    // of a loaded chunk; uncomment this later!!!
    // Stasis_Component::fixed_update();

    Transform::fixed_update();
    Velocity_Component::fixed_update();

    world::fixed_update();
}

void shared_game::fixed_update_late(void)
{
    ZoneScoped;

    // empty
}
