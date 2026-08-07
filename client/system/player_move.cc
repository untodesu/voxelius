#include "client/pch.hh"

#include "client/system/player_move.hh"

#include "core/camera.hh"
#include "core/config/ref.hh"

#include "shared/component/transform.hh"
#include "shared/component/velocity.hh"
#include "shared/world/world.hh"

#include "client/camera.hh"
#include "client/globals.hh"
#include "client/gui/container.hh"
#include "client/gui/keybind.hh"
#include "client/settings.hh"

constexpr static float SPEED = 16.0f;

static gui::KeyBind s_key_forward;
static gui::KeyBind s_key_backward;
static gui::KeyBind s_key_left;
static gui::KeyBind s_key_right;
static gui::KeyBind s_key_jump;
static gui::KeyBind s_key_crouch;

void player_move::init(void)
{
    s_key_forward.set_value(SDLK_W);
    s_key_forward.bind(globals::client_config, "player_move.key_forward");
    settings::keyboard_movement.add_child(s_key_forward, 0);

    s_key_backward.set_value(SDLK_S);
    s_key_backward.bind(globals::client_config, "player_move.key_backward");
    settings::keyboard_movement.add_child(s_key_backward, 1);

    s_key_left.set_value(SDLK_A);
    s_key_left.bind(globals::client_config, "player_move.key_left");
    settings::keyboard_movement.add_child(s_key_left, 2);

    s_key_right.set_value(SDLK_D);
    s_key_right.bind(globals::client_config, "player_move.key_right");
    settings::keyboard_movement.add_child(s_key_right, 3);

    s_key_jump.set_value(SDLK_SPACE);
    s_key_jump.bind(globals::client_config, "player_move.key_jump");
    settings::keyboard_movement.add_child(s_key_jump, 4);

    s_key_crouch.set_value(SDLK_LSHIFT);
    s_key_crouch.bind(globals::client_config, "player_move.key_crouch");
    settings::keyboard_movement.add_child(s_key_crouch, 5);
}

void player_move::fixed_update(void)
{
    if(!globals::registry.valid(globals::player)) {
        return;
    }

    const auto& transform = globals::registry.get<Transform>(globals::player);
    auto& velocity = globals::registry.get<Velocity>(globals::player);

    globals::registry.emplace_or_replace<Transform_Prev>(globals::player, transform);
    globals::registry.emplace_or_replace<Velocity_Prev>(globals::player, velocity);

    if(globals::gui_screen) {
        velocity.value = Eigen::Vector3f::Zero();
        return;
    }

    auto keyboard = SDL_GetKeyboardState(nullptr);

    Eigen::Vector3f wishdir = Eigen::Vector3f::Zero();

    auto forward_scancode = SDL_GetScancodeFromKey(s_key_forward.value(), nullptr);
    auto backward_scancode = SDL_GetScancodeFromKey(s_key_backward.value(), nullptr);
    auto left_scancode = SDL_GetScancodeFromKey(s_key_left.value(), nullptr);
    auto right_scancode = SDL_GetScancodeFromKey(s_key_right.value(), nullptr);
    auto jump_scancode = SDL_GetScancodeFromKey(s_key_jump.value(), nullptr);
    auto crouch_scancode = SDL_GetScancodeFromKey(s_key_crouch.value(), nullptr);

    if(keyboard[forward_scancode]) {
        wishdir.z() -= 1.0f;
    }

    if(keyboard[backward_scancode]) {
        wishdir.z() += 1.0f;
    }

    if(keyboard[left_scancode]) {
        wishdir.x() -= 1.0f;
    }

    if(keyboard[right_scancode]) {
        wishdir.x() += 1.0f;
    }

    if(keyboard[jump_scancode]) {
        wishdir.y() += 1.0f;
    }

    if(keyboard[crouch_scancode]) {
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
