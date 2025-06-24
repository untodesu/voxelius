#include "client/pch.hh"

#include "client/listener.hh"

#include "core/config.hh"
#include "core/constexpr.hh"

#include "shared/dimension.hh"
#include "shared/velocity.hh"

#include "client/camera.hh"
#include "client/const.hh"
#include "client/globals.hh"
#include "client/session.hh"
#include "client/sound.hh"

void listener::update(void)
{
    if(session::is_ingame()) {
        const auto& velocity = globals::dimension->entities.get<VelocityComponent>(globals::player).value;
        const auto& position = camera::position_local;

        alListener3f(AL_POSITION, position.x, position.y, position.z);
        alListener3f(AL_VELOCITY, velocity.x, velocity.y, velocity.z);

        float orientation[6];
        orientation[0] = camera::direction.x;
        orientation[1] = camera::direction.y;
        orientation[2] = camera::direction.z;
        orientation[3] = DIR_UP<float>.x;
        orientation[4] = DIR_UP<float>.y;
        orientation[5] = DIR_UP<float>.z;

        alListenerfv(AL_ORIENTATION, orientation);
    }

    alListenerf(AL_GAIN, cxpr::clamp(sound::volume_master.get_value() * 0.01f, 0.0f, 1.0f));
}
