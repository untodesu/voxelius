#ifndef CLIENT_CAMERA_HH
#define CLIENT_CAMERA_HH 1
#pragma once

#include "shared/types.hh"

class ConfigFloat;
class ConfigUnsigned;

namespace camera
{
extern ConfigFloat roll_angle;
extern ConfigFloat vertical_fov;
extern ConfigUnsigned view_distance;
} // namespace camera

namespace camera
{
extern glm::fvec3 angles;
extern glm::fvec3 direction;
extern glm::fmat4x4 matrix;
extern chunk_pos position_chunk;
extern glm::fvec3 position_local;
} // namespace camera

namespace camera
{
void init(void);
void update(void);
} // namespace camera

#endif // CLIENT_CAMERA_HH
