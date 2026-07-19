#include "client/pch.hh"

#include "client/entity/player_move.hh"

#include "core/camera.hh"
#include "core/config/ref.hh"

#include "shared/entity/head.hh"
#include "shared/entity/transform.hh"
#include "shared/entity/velocity.hh"
#include "shared/world/world.hh"

#include "client/camera.hh"
#include "client/globals.hh"
#include "client/gui/gui.hh"
#include "client/gui/settings.hh"

constexpr static float SPEED = 16.0f;

static config::Ref<SDL_Keycode> s_key_forward { SDLK_W };
static config::Ref<SDL_Keycode> s_key_backward { SDLK_S };
static config::Ref<SDL_Keycode> s_key_left { SDLK_A };
static config::Ref<SDL_Keycode> s_key_right { SDLK_D };
static config::Ref<SDL_Keycode> s_key_jump { SDLK_SPACE };
static config::Ref<SDL_Keycode> s_key_crouch { SDLK_LSHIFT };

void player_move::init(void)
{
    s_key_forward.bind(globals::client_config, "player_move.key_forward");
    s_key_backward.bind(globals::client_config, "player_move.key_backward");
    s_key_left.bind(globals::client_config, "player_move.key_left");
    s_key_right.bind(globals::client_config, "player_move.key_right");
    s_key_jump.bind(globals::client_config, "player_move.key_jump");
    s_key_crouch.bind(globals::client_config, "player_move.key_crouch");

    settings::keybind(0, settings_location::KEYBOARD_MOVEMENT, "player_move.key_forward", false);
    settings::keybind(1, settings_location::KEYBOARD_MOVEMENT, "player_move.key_backward", false);
    settings::keybind(2, settings_location::KEYBOARD_MOVEMENT, "player_move.key_left", false);
    settings::keybind(3, settings_location::KEYBOARD_MOVEMENT, "player_move.key_right", false);
    settings::keybind(4, settings_location::KEYBOARD_MOVEMENT, "player_move.key_jump", false);
    settings::keybind(5, settings_location::KEYBOARD_MOVEMENT, "player_move.key_crouch", false);
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
