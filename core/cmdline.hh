#ifndef CORE_CMDLINE_HH
#define CORE_CMDLINE_HH 1
#pragma once

namespace cmdline
{
void create(int argc, char **argv);
void insert(const char *option, const char *argument = nullptr);
const char *get(const char *option, const char *fallback = nullptr);
bool contains(const char *option);
} // namespace cmdline

#endif /* CORE_CMDLINE_HH */
