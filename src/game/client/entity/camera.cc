// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: camera.cc
// Description: Client view matrix calculations

#include "client/pch.hh"

#include "client/entity/camera.hh"

#include "core/config/number.hh"

#include "core/io/config_map.hh"

#include "core/math/angles.hh"

#include "shared/entity/grounded.hh"
#include "shared/entity/head.hh"
#include "shared/entity/transform.hh"
#include "shared/entity/velocity.hh"

#include "shared/world/dimension.hh"

#include "client/entity/player_move.hh"

#include "client/gui/settings.hh"

#include "client/const.hh"
#include "client/globals.hh"
#include "client/session.hh"
#include "client/toggles.hh"

config::Float camera::roll_angle(2.0f, 0.0f, 4.0f);
config::Float camera::vertical_fov(90.0f, 45.0f, 110.0f);
config::Unsigned camera::view_distance(16U, 4U, 32U);

glm::fvec3 camera::angles;
glm::fvec3 camera::direction;
glm::fmat4x4 camera::matrix;
chunk_pos camera::position_chunk;
glm::fvec3 camera::position_local;

static void reset_camera(void)
{
    camera::angles = glm::fvec3(0.0f, 0.0f, 0.0f);
    camera::direction = DIR_FORWARD<float>;
    camera::matrix = glm::identity<glm::fmat4x4>();
    camera::position_chunk = chunk_pos(0, 0, 0);
    camera::position_local = glm::fvec3(0.0f, 0.0f, 0.0f);
}

// Gracefully contributed by PQCraft himself in 2024
// making PlatinumSrc and Voxelius kind of related to each other
static glm::fmat4x4 platinumsrc_viewmatrix(const glm::fvec3& position, const glm::fvec3& angles)
{
    glm::fvec3 forward, up;
    math::vectors(angles, &forward, nullptr, &up);

    auto result = glm::identity<glm::fmat4x4>();
    result[0][0] = forward.y * up.z - forward.z * up.y;
    result[1][0] = forward.z * up.x - forward.x * up.z;
    result[2][0] = forward.x * up.y - forward.y * up.x;
    result[3][0] = -result[0][0] * position.x - result[1][0] * position.y - result[2][0] * position.z;
    result[0][1] = up.x;
    result[1][1] = up.y;
    result[2][1] = up.z;
    result[3][1] = -up.x * position.x - up.y * position.y - up.z * position.z;
    result[0][2] = -forward.x;
    result[1][2] = -forward.y;
    result[2][2] = -forward.z;
    result[3][2] = forward.x * position.x + forward.y * position.y + forward.z * position.z;
    return result;
}

void camera::init(void)
{
    globals::client_config.add_value("camera.roll_angle", camera::roll_angle);
    globals::client_config.add_value("camera.vertical_fov", camera::vertical_fov);
    globals::client_config.add_value("camera.view_distance", camera::view_distance);

    settings::add_slider(1, camera::vertical_fov, settings_location::GENERAL, "camera.vertical_fov", true, "%.0f");
    settings::add_slider(0, camera::view_distance, settings_location::VIDEO, "camera.view_distance", false);
    settings::add_slider(10, camera::roll_angle, settings_location::VIDEO, "camera.roll_angle", true, "%.01f");

    reset_camera();
}

void camera::update(void)
{
    if(!session::is_ingame()) {
        reset_camera();
        return;
    }

    const auto& head = globals::dimension->entities.get<client::HeadIntr>(globals::player);
    const auto& transform = globals::dimension->entities.get<client::TransformIntr>(globals::player);
    const auto& velocity = globals::dimension->entities.get<client::VelocityIntr>(globals::player);

    camera::angles = transform.angles + head.angles;
    camera::position_chunk = transform.chunk;
    camera::position_local = transform.local + head.offset;

    glm::fvec3 right_vector, up_vector;
    math::vectors(camera::angles, &camera::direction, &right_vector, &up_vector);

    auto client_angles = camera::angles;

    if(!toggles::get(TOGGLE_PM_FLIGHT) && globals::dimension->entities.try_get<Grounded>(globals::player)) {
        client_angles[2] = math::radians(-camera::roll_angle.get_value() * glm::dot(velocity.value / PMOVE_MAX_SPEED_GROUND, right_vector));
    }

    const auto z_near = 0.01f;
    const auto z_far = 1.25f * static_cast<float>(CHUNK_SIZE * camera::view_distance.get_value());

    auto proj = glm::perspective(math::radians(camera::vertical_fov.get_value()), globals::aspect, z_near, z_far);
    auto view = platinumsrc_viewmatrix(camera::position_local, client_angles);

    camera::matrix = proj * view;
}
