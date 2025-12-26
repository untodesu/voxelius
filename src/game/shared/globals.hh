// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: globals.hh
// Description: Global variables

#ifndef SHARED_GLOBALS_HH
#define SHARED_GLOBALS_HH
#pragma once

namespace globals
{
extern entt::dispatcher dispatcher;
} // namespace globals

namespace globals
{
extern float fixed_frametime;
extern float fixed_frametime_avg;
extern std::uint64_t fixed_frametime_us;
extern std::size_t fixed_framecount;
} // namespace globals

namespace globals
{
extern std::uint64_t curtime;
} // namespace globals

#endif
