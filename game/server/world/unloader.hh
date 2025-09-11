#pragma once

namespace world
{
class Dimension;
} // namespace world

namespace world::unloader
{
void init(void);
void init_late(void);
void fixed_update_late(Dimension* dimension);
} // namespace world::unloader
