#include "client/pch.hh"

#include "client/fog.hh"

#include "client/constant.hh"
#include "client/entity/camera.hh"
#include "client/world/skybox.hh"

float fog::distance;
Eigen::Vector3f fog::color;

void fog::update(void)
{
    distance = static_cast<float>(constant::CHUNK_SIZE * camera::view_distance.value());
    color = skybox::fog_color; // TODO: fix
}
