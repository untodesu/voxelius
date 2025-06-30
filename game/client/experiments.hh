#ifndef CLIENT_EXPERIMENTS_HH
#define CLIENT_EXPERIMENTS_HH 1
#pragma once

namespace experiments
{
void init(void);
void init_late(void);
void shutdown(void);
void update(void);
void update_late(void);
} // namespace experiments

namespace experiments
{
void attack(void);
void interact(void);
} // namespace experiments

#endif // CLIENT_EXPERIMENTS_HH
