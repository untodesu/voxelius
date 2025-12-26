#pragma once

#include "shared/types.hh"

namespace config
{
class Float;
class Unsigned;
} // namespace config

namespace camera
{
extern config::Float roll_angle;
extern config::Float vertical_fov;
extern config::Unsigned view_distance;
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
