#ifndef SERVER_UNLOADER_HH
#define SERVER_UNLOADER_HH 1
#pragma once

class Dimension;

namespace unloader
{
void init(void);
void init_late(void);
void fixed_update_late(Dimension* dimension);
} // namespace unloader

#endif // SERVER_UNLOADER_HH
