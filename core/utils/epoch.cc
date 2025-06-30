#include "core/pch.hh"

#include "core/utils/epoch.hh"

std::uint64_t utils::unix_seconds(void)
{
    const auto elapsed = std::chrono::system_clock::now().time_since_epoch();
    return static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::seconds>(elapsed).count());
}

std::uint64_t utils::unix_milliseconds(void)
{
    const auto elapsed = std::chrono::system_clock::now().time_since_epoch();
    return static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count());
}

std::uint64_t utils::unix_microseconds(void)
{
    const auto elapsed = std::chrono::system_clock::now().time_since_epoch();
    return static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count());
}

std::int64_t utils::signed_unix_seconds(void)
{
    const auto elapsed = std::chrono::system_clock::now().time_since_epoch();
    return static_cast<std::int64_t>(std::chrono::duration_cast<std::chrono::seconds>(elapsed).count());
}

std::int64_t utils::signed_unix_milliseconds(void)
{
    const auto elapsed = std::chrono::system_clock::now().time_since_epoch();
    return static_cast<std::int64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count());
}

std::int64_t utils::signed_unix_microseconds(void)
{
    const auto elapsed = std::chrono::system_clock::now().time_since_epoch();
    return static_cast<std::int64_t>(std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count());
}
