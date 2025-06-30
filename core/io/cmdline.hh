#ifndef CORE_IO_CMDLINE_HH
#define CORE_IO_CMDLINE_HH 1
#pragma once

namespace io::cmdline
{
void create(int argc, char** argv);
void insert(const char* option, const char* argument = nullptr);
const char* get(const char* option, const char* fallback = nullptr);
bool contains(const char* option);
} // namespace io::cmdline

#endif // CORE_IO_CMDLINE_HH
