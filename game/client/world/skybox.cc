#include "client/pch.hh"

#include "client/world/skybox.hh"

glm::fvec3 world::skybox::fog_color;

void world::skybox::init(void)
{
    // https://convertingcolors.com/hex-color-B1F3FF.html
    world::skybox::fog_color = glm::fvec3(0.690f, 0.950f, 1.000f);
}
