#include "client/pch.hh"

#include "client/camera.hh"

#include "core/camera.hh"
#include "core/config/ref.hh"
#include "core/utils/angles.hh"

#include "shared/head.hh"
#include "shared/transform.hh"
#include "shared/world.hh"

#include "client/constant.hh"
#include "client/globals.hh"
#include "client/settings.hh"

config::Ref<float> camera::vertical_fov { 70.0f };
config::Ref<unsigned> camera::view_distance { 8 };

Camera camera::instance;

Eigen::Vector3f camera::angles;
Eigen::Vector3f camera::forward;
Eigen::Vector3f camera::right;
Eigen::Vector3f camera::up;
Eigen::Vector3f camera::local;
chunk_pos camera::chunk;

static void reset_camera(void)
{
    camera::angles.setZero();
    camera::forward = Eigen::Vector3f::UnitZ();
    camera::right = Eigen::Vector3f::UnitX();
    camera::up = Eigen::Vector3f::UnitY();
    camera::local.setZero();
    camera::chunk.setZero();
}

void camera::init(void)
{
    vertical_fov.bind(globals::client_config, "camera.vertical_fov");
    view_distance.bind(globals::client_config, "camera.view_distance");

    settings::slider<float>(1, settings_location::GENERAL, "camera.vertical_fov", 45.0f, 110.0f, true, "%.0f");
    settings::slider<unsigned>(0, settings_location::VIDEO, "camera.view_distance", 1, 16, false);

    reset_camera();
}

void camera::update(void)
{
    if(world::basic_entities.valid(globals::player)) {
        const auto& head = world::basic_entities.get<Head_Intr>(globals::player);
        const auto& transform = world::basic_entities.get<Transform_Intr>(globals::player);

        camera::angles = head.angles;
        camera::chunk = transform.chunk;
        camera::local = transform.local + head.offset;

        auto z_near = 0.01f;
        auto z_far = 1.25f * static_cast<float>(constant::CHUNK_SIZE * view_distance.value());

        instance.set_perspective(utils::radians(vertical_fov.value()), globals::aspect, z_near, z_far);
        instance.set_view(camera::local, camera::angles);
        instance.update();

        forward = instance.forward_vector();
        right = instance.right_vector();
        up = instance.up_vector();
    }
    else {
        reset_camera();
    }
}
