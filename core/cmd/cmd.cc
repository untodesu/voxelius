#include "core/pch.hh"

#include "core/cmd/cmd.hh"

#include "core/utils/string.hh"

std::unordered_map<std::string, cmd::command_fn> cmd::map;

void cmd::add(std::string_view name, command_fn fn)
{
    map.insert_or_assign(std::string(name), std::move(fn));
}

bool cmd::run(std::string_view name, std::span<std::string_view> args)
{
    auto it = map.find(std::string(name));

    if(it == map.cend()) {
        LOG_WARNING("{}: unknown command", name);

        return false;
    }

    return it->second(args);
}

bool cmd::run(std::string_view command_line)
{
    command_line = utils::remove_comments(command_line);

    auto tokens = utils::tokenize(command_line);

    if(tokens.empty()) {
        return false; // empty command
    }

    std::string_view command_name(tokens[0]);
    std::span<std::string_view> command_args(tokens.data() + 1, tokens.size() - 1);
    return run(command_name, command_args);
}
