#include "client/pch.hh"

#include "client/player_move.hh"

#include "core/camera.hh"
#include "core/config/ref.hh"

#include "shared/head.hh"
#include "shared/transform.hh"
#include "shared/velocity.hh"
#include "shared/world.hh"

#include "client/camera.hh"
#include "client/globals.hh"
#include "client/gui/gui.hh"

constexpr static float SPEED = 16.0f;

void player_move::init(void)
{
    // empty
}

void player_move::fixed_update(void)
{
    if(!world::basic_entities.valid(globals::player)) {
        return;
    }

    const auto& transform = world::basic_entities.get<Transform>(globals::player);
    auto& velocity = world::basic_entities.get<Velocity>(globals::player);

    world::basic_entities.emplace_or_replace<Transform_Prev>(globals::player, transform);
    world::basic_entities.emplace_or_replace<Velocity_Prev>(globals::player, velocity);

    if(gui::screen) {
        velocity.value = Eigen::Vector3f::Zero();
        return;
    }

    auto keyboard = SDL_GetKeyboardState(nullptr);

    Eigen::Vector3f wishdir = Eigen::Vector3f::Zero();

    if(keyboard[SDL_SCANCODE_W]) {
        wishdir.z() -= 1.0f;
    }

    if(keyboard[SDL_SCANCODE_S]) {
        wishdir.z() += 1.0f;
    }

    if(keyboard[SDL_SCANCODE_A]) {
        wishdir.x() -= 1.0f;
    }

    if(keyboard[SDL_SCANCODE_D]) {
        wishdir.x() += 1.0f;
    }

    if(keyboard[SDL_SCANCODE_SPACE]) {
        wishdir.y() += 1.0f;
    }

    if(keyboard[SDL_SCANCODE_LSHIFT]) {
        wishdir.y() -= 1.0f;
    }

    if(wishdir.isZero()) {
        velocity.value = Eigen::Vector3f::Zero();
        return;
    }

    Eigen::Vector3f world_wishdir = camera::instance.orientation() * wishdir;
    world_wishdir.normalize();

    velocity.value = world_wishdir * SPEED;
}

void player_move::update_late(void)
{
    // empty
}
