#include "core/pch.hh"

#include "core/globals.hh"

entt::dispatcher globals::dispatcher;

std::uint64_t globals::curtime_us;

std::size_t globals::fixed_framecount;
std::uint64_t globals::fixed_frametime_us;
float globals::fixed_frametime;
float globals::fixed_frametime_avg;
