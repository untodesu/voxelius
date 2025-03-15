#include "client/pch.hh"
#include "client/skybox.hh"

glm::fvec3 skybox::fog_color;

void skybox::init(void)
{
    // https://convertingcolors.com/hex-color-B1F3FF.html
    skybox::fog_color = glm::fvec3(0.690f, 0.950f, 1.000f);
}
