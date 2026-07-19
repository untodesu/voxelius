#include "client/pch.hh"

#include "client/world/skybox.hh"

Eigen::Vector3f skybox::sky_color;

void skybox::init(void)
{
    // https://convertingcolors.com/hex-color-B1F3FF.html
    sky_color = Eigen::Vector3f(0.690f, 0.950f, 1.000f);
}
