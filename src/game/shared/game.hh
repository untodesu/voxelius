#pragma once

namespace shared_game
{
void init(int argc, char** argv, std::string_view logfile = {});
void shutdown(void);
} // namespace shared_game
