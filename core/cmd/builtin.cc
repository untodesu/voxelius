#include "core/pch.hh"

#include "core/cmd/builtin.hh"

#include "core/version.hh"

bool cmd::builtin::cmd_version(std::span<std::string_view> args) noexcept
{
    LOG_INFO("voxelius {}", version::full);

    return true;
}
