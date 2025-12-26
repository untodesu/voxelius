// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: experiments.hh
// Description: Experimental things that are still haven't got their dedicated game system

#ifndef CLIENT_EXPERIMENTS_HH
#define CLIENT_EXPERIMENTS_HH
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

#endif
