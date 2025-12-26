#pragma once

class Dimension;

namespace unloader
{
void init(void);
void init_late(void);
void fixed_update_late(Dimension* dimension);
} // namespace unloader
