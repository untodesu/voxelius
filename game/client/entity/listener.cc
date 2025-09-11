#include "client/pch.hh"

#include "client/entity/listener.hh"

#include "core/config/number.hh"

#include "core/math/constexpr.hh"

#include "shared/entity/velocity.hh"

#include "shared/world/dimension.hh"

#include "client/entity/camera.hh"

#include "client/sound/sound.hh"

#include "client/const.hh"
#include "client/globals.hh"
#include "client/session.hh"

void entity::listener::update(void)
{
    if(session::is_ingame()) {
        const auto& velocity = globals::dimension->entities.get<entity::Velocity>(globals::player).value;
        const auto& position = entity::camera::position_local;

        alListener3f(AL_POSITION, position.x, position.y, position.z);
        alListener3f(AL_VELOCITY, velocity.x, velocity.y, velocity.z);

        float orientation[6];
        orientation[0] = entity::camera::direction.x;
        orientation[1] = entity::camera::direction.y;
        orientation[2] = entity::camera::direction.z;
        orientation[3] = DIR_UP<float>.x;
        orientation[4] = DIR_UP<float>.y;
        orientation[5] = DIR_UP<float>.z;

        alListenerfv(AL_ORIENTATION, orientation);
    }

    alListenerf(AL_GAIN, math::clamp(sound::volume_master.get_value() * 0.01f, 0.0f, 1.0f));
}
