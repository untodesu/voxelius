#include "client/pch.hh"
#include "client/player_move.hh"

#include "core/angles.hh"
#include "core/config.hh"
#include "core/constexpr.hh"

#include "shared/dimension.hh"
#include "shared/grounded.hh"
#include "shared/head.hh"
#include "shared/transform.hh"
#include "shared/velocity.hh"

#include "client/const.hh"
#include "client/gamepad_axis.hh"
#include "client/gamepad_button.hh"
#include "client/gamepad.hh"
#include "client/globals.hh"
#include "client/gui_screen.hh"
#include "client/keybind.hh"
#include "client/session.hh"
#include "client/settings.hh"
#include "client/sound.hh"
#include "client/status_lines.hh"
#include "client/toggles.hh"
#include "client/voxel_sounds.hh"

constexpr static std::uint64_t PMOVE_JUMP_COOLDOWN = 500000; // 0.5 seconds

constexpr static float PMOVE_FOOTSTEP_SIZE = 2.0f;

// Movement keys
static ConfigKeyBind key_move_forward(GLFW_KEY_W);
static ConfigKeyBind key_move_back(GLFW_KEY_S);
static ConfigKeyBind key_move_left(GLFW_KEY_A);
static ConfigKeyBind key_move_right(GLFW_KEY_D);
static ConfigKeyBind key_move_down(GLFW_KEY_LEFT_SHIFT);
static ConfigKeyBind key_move_up(GLFW_KEY_SPACE);

// Movement gamepad axes
static ConfigGamepadAxis axis_move_forward(GLFW_GAMEPAD_AXIS_RIGHT_X, false);
static ConfigGamepadAxis axis_move_sideways(GLFW_GAMEPAD_AXIS_RIGHT_Y, false);

// Movement gamepad buttons
static ConfigGamepadButton button_move_down(GLFW_GAMEPAD_BUTTON_DPAD_DOWN);
static ConfigGamepadButton button_move_up(GLFW_GAMEPAD_BUTTON_DPAD_UP);

// General movement options
static ConfigBoolean enable_speedometer(true);

static glm::fvec3 movement_direction;

static std::uint64_t next_jump_us;
static float speedometer_value;
static float footsteps_distance;

static std::mt19937_64 pitch_random;
static std::uniform_real_distribution<float> pitch_distrib;

// Quake III's PM_Accelerate-ish function used for
// conventional (gravity-affected non-flight) movement
static glm::fvec3 pm_accelerate(const glm::fvec3 &wishdir, const glm::fvec3 &velocity, float wishspeed, float accel)
{
    auto current_speed = glm::dot(velocity, wishdir);
    auto add_speed = wishspeed - current_speed;

    if(add_speed <= 0.0f) {
        // Not accelerating
        return velocity;
    }

    auto accel_speed = cxpr::min(add_speed, accel * globals::fixed_frametime * wishspeed);

    auto result = glm::fvec3(velocity);
    result.x += accel_speed * wishdir.x;
    result.z += accel_speed * wishdir.z;
    return result;
}

// Conventional movement - velocity update when not on the ground
static glm::fvec3 pm_air_move(const glm::fvec3 &wishdir, const glm::fvec3 &velocity)
{
    return pm_accelerate(wishdir, velocity, PMOVE_ACCELERATION_AIR, PMOVE_MAX_SPEED_AIR);
}

// Conventional movement - velocity uodate when on the ground
static glm::fvec3 pm_ground_move(const glm::fvec3 &wishdir, const glm::fvec3 &velocity)
{
    if(auto speed = glm::length(velocity)) {
        auto speed_drop = speed * PMOVE_FRICTION_GROUND * globals::fixed_frametime;
        auto speed_factor = cxpr::max(speed - speed_drop, 0.0f) / speed;
        return pm_accelerate(wishdir, velocity * speed_factor, PMOVE_ACCELERATION_GROUND, PMOVE_MAX_SPEED_GROUND);
    }

    return pm_accelerate(wishdir, velocity, PMOVE_ACCELERATION_GROUND, PMOVE_MAX_SPEED_GROUND);
}

// A simpler minecraft-like acceleration model
// used whenever the TOGGLE_PM_FLIGHT is enabled
static glm::fvec3 pm_flight_move(const glm::fvec3 &wishdir)
{
    // FIXME: make it smoother
    return wishdir * PMOVE_MAX_SPEED_AIR;
}

void player_move::init(void)
{
    movement_direction = ZERO_VEC3<float>;

    next_jump_us = 0x0000000000000000U;
    speedometer_value = 0.0f;
    footsteps_distance = 0.0f;

    // UNDONE: make this a separate subsystem
    pitch_random.seed(std::random_device()());
    pitch_distrib = std::uniform_real_distribution<float>(0.9f, 1.1f);

    globals::client_config.add_value("player_move.key.forward", key_move_forward);
    globals::client_config.add_value("player_move.key.back", key_move_back);
    globals::client_config.add_value("player_move.key.left", key_move_left);
    globals::client_config.add_value("player_move.key.right", key_move_right);
    globals::client_config.add_value("player_move.key.down", key_move_down);
    globals::client_config.add_value("player_move.key.up", key_move_up);
    globals::client_config.add_value("player_move.gp_axis.move_forward", axis_move_forward);
    globals::client_config.add_value("player_move.gp_axis.move_sideways", axis_move_sideways);
    globals::client_config.add_value("player_move.gp_button.move_down", button_move_down);
    globals::client_config.add_value("player_move.gp_button.move_up", button_move_up);
    globals::client_config.add_value("player_move.enable_speedometer", enable_speedometer);

    settings::add_keybind(1, key_move_forward, settings_location::KEYBOARD_MOVEMENT, "player_move.key.forward");
    settings::add_keybind(2, key_move_back, settings_location::KEYBOARD_MOVEMENT, "player_move.key.back");
    settings::add_keybind(3, key_move_left, settings_location::KEYBOARD_MOVEMENT, "player_move.key.left");
    settings::add_keybind(4, key_move_right, settings_location::KEYBOARD_MOVEMENT, "player_move.key.right");
    settings::add_keybind(5, key_move_down, settings_location::KEYBOARD_MOVEMENT, "player_move.key.down");
    settings::add_keybind(6, key_move_up, settings_location::KEYBOARD_MOVEMENT, "player_move.key.up");

    settings::add_gamepad_axis(0, axis_move_forward, settings_location::GAMEPAD_MOVEMENT, "player_move.gp_axis.move_forward");
    settings::add_gamepad_axis(1, axis_move_sideways, settings_location::GAMEPAD_MOVEMENT, "player_move.gp_axis.move_sideways");
    settings::add_gamepad_button(2, button_move_down, settings_location::GAMEPAD_MOVEMENT, "player_move.gp_button.move_down");
    settings::add_gamepad_button(3, button_move_up, settings_location::GAMEPAD_MOVEMENT, "player_move.gp_button.move_up");

    settings::add_checkbox(2, enable_speedometer, settings_location::VIDEO_GUI, "player_move.enable_speedometer", true);
}

void player_move::fixed_update(void)
{
    const auto &head = globals::dimension->entities.get<HeadComponent>(globals::player);
    auto &transform = globals::dimension->entities.get<TransformComponent>(globals::player);
    auto &velocity = globals::dimension->entities.get<VelocityComponent>(globals::player);

    // Interpolation - preserve current component states
    globals::dimension->entities.emplace_or_replace<TransformComponentPrev>(globals::player, transform);

    glm::fvec3 forward, right;
    cxangles::vectors(glm::fvec3(0.0f, head.angles[1], 0.0f), &forward, &right, nullptr);

    glm::fvec3 wishdir = ZERO_VEC3<float>;
    glm::fvec3 movevars = glm::fvec3(movement_direction.x, 0.0f, movement_direction.z);
    wishdir.x = glm::dot(movevars, right);
    wishdir.z = glm::dot(movevars, forward);

    if(toggles::get(TOGGLE_PM_FLIGHT)) {
        velocity.value = pm_flight_move(glm::fvec3(wishdir.x, movement_direction.y, wishdir.z));
        return;
    }

    auto grounded = globals::dimension->entities.try_get<GroundedComponent>(globals::player);
    auto velocity_horizontal = glm::fvec3(velocity.value.x, 0.0f, velocity.value.z);

    if(grounded) {
        auto new_velocity = pm_ground_move(wishdir, velocity_horizontal);
        velocity.value.x = new_velocity.x;
        velocity.value.z = new_velocity.z;

        auto new_speed = glm::length(new_velocity);

        if(new_speed > 0.01f)
            footsteps_distance += globals::fixed_frametime * new_speed;
        else footsteps_distance = 0.0f;

        if(footsteps_distance >= PMOVE_FOOTSTEP_SIZE) {
            if(auto effect = voxel_sounds::get_footsteps(grounded->surface))
                sound::play_player(effect, false, pitch_distrib(pitch_random));
            footsteps_distance = 0.0f;
        }
    }
    else {
        auto new_velocity = pm_air_move(wishdir, velocity_horizontal);
        velocity.value.x = new_velocity.x;
        velocity.value.z = new_velocity.z;
    }

    if(movement_direction.y == 0.0f) {
        // Allow players to queue bunny-jumps by quickly
        // releasing and pressing the jump key again without a cooldown
        next_jump_us = 0x0000000000000000U;
        return;
    }

    if(grounded && (movement_direction.y > 0.0f) && (globals::curtime >= next_jump_us)) {
        velocity.value.y = -PMOVE_JUMP_FORCE * globals::dimension->get_gravity();

        auto new_speed = glm::length(glm::fvec2(velocity.value.x, velocity.value.z));
        auto new_speed_text = fmt::format("{:.02f} M/S", new_speed);
        auto speed_change = new_speed - speedometer_value;

        speedometer_value = new_speed;

        next_jump_us = globals::curtime + PMOVE_JUMP_COOLDOWN;

        if(enable_speedometer.get_value()) {
            if(cxpr::abs(speed_change) < 0.01f) {
                // No considerable speed increase within
                // the precision we use to draw the speedometer
                status_lines::set(STATUS_DEBUG, new_speed_text, ImVec4(0.7f, 0.7f, 0.7f, 1.0f), 1.0f);
            }
            else if(speed_change < 0.0f) {
                // Speed change is negative, we are actively
                // slowing down; use the red color for the status line
                status_lines::set(STATUS_DEBUG, new_speed_text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f), 1.0f);
            }
            else {
                // Speed change is positive, we are actively
                // speeding up; use the green color for the status line
                status_lines::set(STATUS_DEBUG, new_speed_text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f), 1.0f);
            }
        }

        if(auto effect = voxel_sounds::get_footsteps(grounded->surface)) {
            sound::play_player(effect, false, 1.0f);
        }
    }
}

void player_move::update_late(void)
{
    movement_direction = ZERO_VEC3<float>;

    if(globals::gui_screen || !session::is_ingame()) {
        // We're either disconnected or have the
        // UI opened up; anyways we shouldn't move
        return;
    }

    if(gamepad::available && gamepad::active.get_value()) {
        if(button_move_down.is_pressed(gamepad::state))
            movement_direction += DIR_DOWN<float>;
        if(button_move_up.is_pressed(gamepad::state))
            movement_direction += DIR_UP<float>;
        movement_direction.x += axis_move_sideways.get_value(gamepad::state, gamepad::deadzone.get_value());
        movement_direction.z -= axis_move_forward.get_value(gamepad::state, gamepad::deadzone.get_value());
    }
    else {
        if(GLFW_PRESS == glfwGetKey(globals::window, key_move_forward.get_key()))
            movement_direction += DIR_FORWARD<float>;
        if(GLFW_PRESS == glfwGetKey(globals::window, key_move_back.get_key()))
            movement_direction += DIR_BACK<float>;
        if(GLFW_PRESS == glfwGetKey(globals::window, key_move_left.get_key()))
            movement_direction += DIR_LEFT<float>;
        if(GLFW_PRESS == glfwGetKey(globals::window, key_move_right.get_key()))
            movement_direction += DIR_RIGHT<float>;
        if(GLFW_PRESS == glfwGetKey(globals::window, key_move_down.get_key()))
            movement_direction += DIR_DOWN<float>;
        if(GLFW_PRESS == glfwGetKey(globals::window, key_move_up.get_key()))
            movement_direction += DIR_UP<float>;
    }
}
